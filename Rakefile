require 'fileutils'

def execute(command, *args)
	#puts [command, *args].join(' ')
	IO.popen([command, *args]) do |f|
		print f.read
	end
	raise "#{command} failed with error code #{$?.exitstatus}" if $?.exitstatus != 0
end

kernel_binary = 'build/kernel.elf'
	
desc "Build Avery"
task :build do
	kernel_bitcode = 'build/kernel.bc'
	kernel_object = 'build/kernel.o'
	sources = Dir['src/**/*']
	bitcodes = []
	objects = ['font.o', kernel_object]
	
	sources.map do |source|
		ext = File.extname(source)
		case File.extname(source)
			when '.S'
				puts "Assembling #{source}..."
				assembly = "build/#{source}.s"
				object_file = "build/#{source}.o"
				FileUtils.makedirs(File.dirname(assembly))
				execute 'clang', '-E', source, '-o', assembly
				execute 'x86_64-elf-as', assembly, '-o', object_file
				
				objects << object_file
			when '.cpp'
				puts "Compiling #{source}..."
				bitcode = "build/#{source}.o"
				FileUtils.makedirs(File.dirname(bitcode))
				execute 'clang', '-std=gnu++11', '-target', 'x86_64-generic-generic', '-emit-llvm', '-c', '-ffreestanding', '-Wall', '-Wextra', '-fno-rtti', '-fno-exceptions', '-fno-unwind-tables', '-fno-inline', source, '-o', bitcode
				bitcodes << bitcode
		end
	end
	
	puts "Linking..."
	
	execute 'llvm-link', *bitcodes, "-o=#{kernel_bitcode}"
	execute 'llc', kernel_bitcode, '-filetype=obj', '-disable-red-zone', '-code-model=kernel', '-relocation-model=static', '-mattr=-sse,-sse2,-mmx', '-O2', '-o', kernel_object
	execute 'x86_64-elf-ld', '-z', 'max-page-size=0x1000', '-T', 'src/x86_64/kernel.ld', *objects, '-o', kernel_binary
end

desc "Test Avery with QEMU"
task :test do
	Dir.chdir('emu/') do
		FileUtils.cp "../#{kernel_binary }", "hda/efi/boot"
		puts "Running QEMU..."
		execute *%w{qemu/qemu-system-x86_64 -L . -bios OVMF.fd -hda fat:hda -serial file:serial.txt -d int,cpu_reset -no-reboot -s}
	end
end

task :run => [:build, :run]

task :default => :build
