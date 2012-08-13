require 'fileutils'
require 'lokar'
require_relative 'rake/build'

def preprocess(input, output, binding)
	content = File.open(input, 'r') { |f| f.read }
	output_content = Lokar.render(content, input, binding)
	File.open(output, 'w') { |f| f.write output_content }
end

def assemble(build, source, objects)
	object_file = source.output(".o")
	
	build.process object_file, source.path do
		build.execute 'x86_64-elf-as', source.path, '-o', object_file
	end
	
	objects << object_file
end

def bitcode_link(build, object, bitcode, bitcodes, options)
	build.process object, *bitcodes do
		build.execute 'llvm-link', *bitcodes, "-o=#{bitcode}"
		build.execute 'llc', bitcode, '-filetype=obj', *options, '-relocation-model=static', '-disable-fp-elim', '-mattr=-sse,-sse2,-mmx', '-O2', '-o', object
	end
end

def analyze(pattern, x86_32 = nil)
	files = Dir[pattern]
	x86_32_files = Dir[x86_32] if x86_32
	
	files.each do |file|
		next if Dir.exists? file
		next if File.extname(file) != '.cpp'
		
		puts "Analyzing #{file}..."
		
		target = 'x86_64-generic-generic'
		target = 'i386-generic-generic' if x86_32 && x86_32_files.include?(file)
		
		Build.execute 'clang', '-target', *target, '-std=gnu++11', '-S', '--analyze', file
	end
end

task :analyze do
	analyze('src/**/*', 'src/x86_64/multiboot/bootstrap/**/*')
end

build_user = proc do
	build = Build.new('build', 'usr_info.yml')
	
	sources = build.package('usr/**/*')
	user_bitcode = build.output "usr.bc"
	user_object = build.output "usr.o"
	user_binary = build.output "usr.elf"
	
	bitcodes = []
	objects = [user_object]
	
	build.run do
		sources.each do |source|
			case source.ext
				when '.S'
					assemble(build, source, objects)
				when '.cpp', '.c'
					bitcode = source.output(".o")
					
					build.cpp(source)
					build.process bitcode, source.path do
						build.execute 'clang', '-target', 'x86_64-generic-generic', '-std=gnu++11', '-emit-llvm', '-c', '-ffreestanding', '-Wall', '-Wextra', '-fno-rtti', '-fno-exceptions', '-fno-unwind-tables', '-fno-inline', source.path, '-o', bitcode
					end
					
					bitcodes << bitcode
			end
		end
	
		puts "Linking..."
		
		bitcode_link(build, user_object, build.output("usr.bc"), bitcodes, ['-code-model=large'])
		
		build.process user_binary, *objects do
			build.execute 'x86_64-elf-ld', '-z', 'max-page-size=0x1000', '-T', 'usr/link.ld', *objects, '-o', user_binary
		end
		
		build.execute 'bin\mcopy', '-D', 'o', '-D', 'O', '-i' ,'emu/grubdisk.img@@1M', user_binary, '::usr.elf'
		FileUtils.cp user_binary, "emu/hda/efi/boot"
	end
end

type = :multiboot
build_kernel = proc do
	build = Build.new('build', 'info.yml')
	kernel_binary = build.output "#{type}/kernel.elf"
	kernel_bitcode_bootstrap = build.output "#{type}/bootstrap.bc"
	kernel_object = build.output "#{type}/kernel.o"
	kernel_assembly_bootstrap = build.output "#{type}/bootstrap.s"
	kernel_object_bootstrap = build.output "#{type}/bootstrap.o"
	
	sources = build.package('src/**/*')
	
	boot_files = sources.extract('src/x86_64/boot/**/*')
	multiboot_files = sources.extract('src/x86_64/multiboot/**/*')
	multiboot_bootstrap_files = multiboot_files.extract('src/x86_64/multiboot/bootstrap/**/*')
	
	if type == :multiboot
		sources.add multiboot_files
		sources.add multiboot_bootstrap_files
	else
		sources.add boot_files
	end
	
	bitcodes = []
	bitcodes_bootstrap = []
	objects = ['font.o', kernel_object]
	
	build.run do
		sources.each do |source|
			case source.ext
				when '.s'
					assemble(build, source, objects)
				when '.cpp', '.c'
					options = ['-target', 'x86_64-generic-generic', '-g']
					
					bootstrap = multiboot_bootstrap_files.include? source
					
					options = ['-target', 'i386-generic-generic'] if bootstrap
					
					bitcode = source.output(".o")
					
					build.cpp(source)
					build.process bitcode, source.path do
						build.execute 'clang', *options, '-std=gnu++11', '-emit-llvm', '-c', '-fno-omit-frame-pointer', '-mno-red-zone', '-ffreestanding', '-Wall', '-Wextra', '-fno-rtti', '-fno-exceptions', '-fno-unwind-tables', '-fno-inline', source.path, '-o', bitcode
					end
					
					if bootstrap
						bitcodes_bootstrap << bitcode
					else
						bitcodes << bitcode
					end
			end
		end
		
		puts "Linking..."
		
		if type == :multiboot
			build.process kernel_object_bootstrap, *bitcodes_bootstrap do
				build.execute 'llvm-link', *bitcodes_bootstrap, "-o=#{kernel_bitcode_bootstrap}"
				build.execute 'llc', kernel_bitcode_bootstrap, '-filetype=asm', '-mattr=-sse,-sse2,-mmx', '-O2', '-o', kernel_assembly_bootstrap
				File.open(kernel_assembly_bootstrap, 'r+') do |file|
					content = file.read
					file.pos = 0
					file.write ".code32\n"
					file.write content
				end
				build.execute 'x86_64-elf-as', kernel_assembly_bootstrap, '-o', kernel_object_bootstrap
			end
			
			objects << kernel_object_bootstrap
		end
		
		kernel_linker_script = build.output "#{type}/kernel.ld"
	
		build.process kernel_linker_script, 'src/x86_64/kernel.ld' do |o, i|
			multiboot = type == :multiboot
			preprocess(i, kernel_linker_script, binding)
		end
		
		bitcode_link(build, kernel_object, build.output("#{type}/kernel.bc"), bitcodes, ['-disable-red-zone', '-code-model=kernel'])
		
		build.process kernel_binary, *objects, kernel_linker_script do
			build.execute 'x86_64-elf-ld', '-z', 'max-page-size=0x1000', '-T', kernel_linker_script, *objects, '-o', kernel_binary
		end
		
		case type
			when :multiboot
				build.execute 'bin\mcopy', '-D', 'o', '-D', 'O', '-i' ,'emu/grubdisk.img@@1M', kernel_binary, '::kernel.elf'
			when :boot
				FileUtils.cp kernel_binary, "emu/hda/efi/boot"
		end
	end
end

task :user do
	build_user.call
end

task :build do
	type = :multiboot
	build_kernel.call
end

task :build_boot do
	type = :boot
	build_kernel.call
end

task :qemu => :build do
	Dir.chdir('emu/') do
		puts "Running QEMU..."
		Build.execute *%w{qemu/qemu-system-x86_64 -L qemu\Bios -hda grubdisk.img -serial file:serial.txt -d int,cpu_reset -no-reboot -s -smp 4}
	end
end

task :qemu_efi => :build_boot do
	Dir.chdir('emu/') do
		puts "Running QEMU..."
		Build.execute *%w{qemu/qemu-system-x86_64 -L . -bios OVMF.fd -hda fat:hda -serial file:serial.txt -d int,cpu_reset -no-reboot -s -smp 4}
	end
end

task :bochs => :build do
	
	Dir.chdir('emu/') do
		puts "Running Bochs..."
		Build.execute 'bochs\bochs', '-q', '-f', 'avery.bxrc'
	end
end

task :default => :build
