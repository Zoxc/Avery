require 'yaml'
require 'digest'
require 'pathname'

class Build
	class Path
		attr_reader :path, :depends, :new_digest
		attr_accessor :old_digest, :pending_depends
		
		def initialize(build, path)
			@build = build
			@path = path
			raise unless String === @path
		end
		
		def output(type)
			File.join(@build.dir, path + type)
		end
		
		def ext
			File.extname(@path)
		end
		
		def load_depends
			return unless @pending_depends
			@depends = @pending_depends.map { |d| @build.file(:input, d) }
		end
		
		def depends_outdated?
			result = nil
			begin
				raise "Circular dependencies" if @lock
				@lock = true
				@depends.each { |d| result = d.outdated? || result }
			ensure
				@lock = nil
			end
			result
		end
		
		def digest
			digest = Digest::SHA2.new(256)
			digest.file @path
			digest.hexdigest
		end
		
		def update
			@new_digest ||= digest
		end
	end
	
	class Input < Path
		attr_accessor :depends_generator
		
		def type
			:input
		end
		
		def store_depends
			@depends.map { |d| d.path }
		end
		
		def update_depends
			@old_digest = nil
			@outdated = true
			@depends = []
			@depends_generator.call if @depends_generator
		end
		
		def outdated?
			return @outdated if @outdated != nil
			
			update
			
			if @old_digest
				if @old_digest != @new_digest
					puts "#{path} is outdated"
					update_depends
					depends_outdated?
				else
					update_depends unless @depends
					@outdated = depends_outdated?
				end
			else
				puts "#{path} is new"
				update_depends
				depends_outdated?
			end
			
			@outdated
		end
	end
	
	class Output < Path
		def type
			:output
		end
		
		def store_depends
		end
		
		def add_depends(inputs)
			@depends ||= []
			@depends.concat inputs
			@depends.uniq!
		end
		
		def outdated?
			return @outdated if @outdated != nil
			
			@outdated = !@old_digest
		end
		
		def rebuild?
			result = depends_outdated? || outdated?
			@old_digest = nil if result
			result
		end
		
		def rebuild
			@outdated = true
			update
		end
	end
	
	class Package
		attr_reader :files
		
		def patterns_to_files(patterns)
			files = []
			patterns.each do |pattern|
				files += Dir[pattern]
			end
			files
		end

		def initialize(build, patterns)
			@build = build
			@files = patterns_to_files(patterns).map { |f| @build.file(:input, f) }
		end
		
		def to_a
			@files
		end
		
		def extract(*patterns)
			package = Package.new(@build, patterns)
			@files -= package.files
			package
		end
		
		def each(&block)
			@files.each(&block)
		end
		
		def add(package)
			@files += package.files
		end
		
		def include?(file)
			@files.include? file
		end
	end
	
	attr_reader :dir
		
	TypeMap = {input: Input, output: Output}

	def output(name)
		File.join(@dir, name)
	end
	
	def initialize(dir, storage)
		@dir = dir
		@storage = File.join(@dir, storage)
		@files = {}
		load(YAML.load_file(@storage)) if File.exist? @storage
	end
	
	def file(type, path)
		file = @files[path]
		return file if file
		@files[path] = TypeMap[type].new(self, path)
	end
	
	def load(storage)
		storage.each do |file, info|
			file = file(info['type'].to_sym, file)
			file.old_digest = info['digest']
			
			if info['depends']
				file.pending_depends = info['depends']
			end
		end
		
		@files.values.each(&:load_depends)
	end
	
	def capture(command, *args)
		#puts [command, *args].join(' ')
		result = IO.popen([command, *args]) do |f|
			f.read
		end
		raise "#{command} failed with error code #{$?.exitstatus}" if $?.exitstatus != 0
		result
	end
	
	def self.execute(command, *args)
		#puts [command, *args].join(' ')
		IO.popen([command, *args]) do |f|
			print f.read
		end
		raise "#{command} failed with error code #{$?.exitstatus}" if $?.exitstatus != 0
	end
	
	def execute(*args)
		Build.execute(*args)
	end
	
	def run
		begin
			yield
		ensure
			output = {}
			@files.each do |file, path|
				digest = path.new_digest || path.old_digest
				next unless digest
				data = {'digest' => digest, 'type' => path.type}
				depends = path.store_depends
				data['depends'] = depends if depends
				output[file] = data
			end
			File.open(@storage, 'w') { |f| YAML.dump(output, f) } 
		end
	end
	
	def mkdirs(target)
		FileUtils.makedirs(File.dirname(target))
	end

	def respace(str)
		str.gsub(/#{"__&NBSP;__"}/, ' ')
	end
	
	def process_line(depends, line)
		file_tasks, args = line.split(':')
		return if args.nil?
		dependencies = args.split.map { |d| respace(d) }
		file_tasks.strip.split.each do |file_task|
			file_task = respace(file_task)
			depends.concat dependencies
		end
	end
	
	def cpp(input)
		input.depends_generator = proc do
			mkdirs(input.path)
			
			depends = []
			
			lines = capture 'clang++', '-MM', '-MT', 'out', input.path
			lines.gsub!(/\\ /, "__&NBSP;__")
			lines.gsub!(/#[^\n]*\n/m, "")
			lines.gsub!(/\\\n/, ' ')
			lines.split("\n").each do |line|
				process_line(depends, line)
			end
			
			depends = depends.map { |d| Pathname.new(d).relative_path_from(Pathname.new('.')).to_s }
			
			depends.uniq!
			depends.delete(input.path)
			
			depends.each { |d| input.depends << file(:input, d) }
		end
	end
	
	def process(output, *inputs, &block)
		inputs = [*inputs]
		input_files = inputs.map { |i| file(:input, i) }
		
		output_file = file(:output, output)
		output_file.add_depends(input_files)
		
		if output_file.rebuild?
			puts "Creating #{output}..."
			mkdirs(output)
			block.call output, *inputs
			output_file.rebuild
		end
	end
	
	def package(*patterns)
		Package.new(self, patterns)
	end
end
