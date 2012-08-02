require 'fileutils'

def execute(command, *args)
	puts [command, *args].join(' ')
	IO.popen([command, *args]) do |f|
		print f.read
	end
	raise "#{command} failed with error code #{$?.exitstatus}" if $?.exitstatus != 0
end

kernel_binary = 'build/kernel.elf'
multiboot = true
	
desc "Build Avery"
task :build do
	kernel_bitcode = 'build/kernel.bc'
	kernel_bitcode_bootstrap = 'build/kernel-bootstrap.bc'
	kernel_object = 'build/kernel.o'
	kernel_assembly_bootstrap = 'build/kernel-bootstrap.s'
	kernel_object_bootstrap = 'build/kernel-bootstrap.o'
	
	sources = Dir['src/**/*']
	
	boot_files = Dir['src/x86_64/boot/**/*']
	multiboot_files = Dir['src/x86_64/multiboot/**/*']
	multiboot_bootstrap_files = Dir['src/x86_64/multiboot/bootstrap/**/*']
	
	if multiboot
		sources -= boot_files
	else
		sources -= multiboot_files
	end
	
	bitcodes = []
	bitcodes_bootstrap = []
	objects = ['font.o', kernel_object]
	
	puts sources.inspect
	
	sources.map do |source|
		ext = File.extname(source)
		case ext
			when '.S'
				puts "Assembling #{source}..."
				assembly = "build/#{source}.s"
				object_file = "build/#{source}.o"
				FileUtils.makedirs(File.dirname(assembly))
				execute 'clang', '-E', source, '-o', assembly
				execute 'x86_64-elf-as', assembly, '-o', object_file
				
				objects << object_file
			when '.cpp', '.c'
				options = ['-target', 'x86_64-generic-generic']
				
				bootstrap = multiboot_bootstrap_files.include? source
				
				options = ['-target', 'i386-generic-generic'] if bootstrap
				
				options << '-DMULTIBOOT' if multiboot
				
				puts "Compiling #{source}..."
				bitcode = "build/#{source}.o"
				FileUtils.makedirs(File.dirname(bitcode))
				execute 'clang', *options, '-std=gnu++11', '-emit-llvm', '-c', '-ffreestanding', '-Wall', '-Wextra', '-fno-rtti', '-fno-exceptions', '-fno-unwind-tables', '-fno-inline', source, '-o', bitcode
				
				if bootstrap
					bitcodes_bootstrap << bitcode
				else
					bitcodes << bitcode
				end
		end
	end
	
	puts "Linking..."
	
	if multiboot
		execute 'llvm-link', *bitcodes_bootstrap, "-o=#{kernel_bitcode_bootstrap}"
		execute 'llc', kernel_bitcode_bootstrap, '-filetype=asm', '-mattr=-sse,-sse2,-mmx', '-O2', '-o', kernel_assembly_bootstrap
		File.open(kernel_assembly_bootstrap, 'r+') do |file|
			content = file.read
			file.pos = 0
			file.write ".code32\n"
			file.write content
		end
		execute 'x86_64-elf-as', kernel_assembly_bootstrap, '-o', kernel_object_bootstrap
		objects << kernel_object_bootstrap
	end
	
	execute 'llvm-link', *bitcodes, "-o=#{kernel_bitcode}"
	execute 'llc', kernel_bitcode, '-filetype=obj', '-disable-red-zone', '-code-model=kernel', '-relocation-model=static', '-mattr=-sse,-sse2,-mmx', '-O2', '-o', kernel_object
	execute 'x86_64-elf-ld', '-z', 'max-page-size=0x1000', '-T', 'src/x86_64/kernel.ld', *objects, '-o', kernel_binary
end

desc "Test Avery with QEMU"
task :test do
	Dir.chdir('emu/') do
		FileUtils.cp "../#{kernel_binary}", "hda/efi/boot"
		puts "Running QEMU..."
		execute *%w{qemu/qemu-system-x86_64 -L . -bios OVMF.fd -hda fat:hda -serial file:serial.txt -d int,cpu_reset -no-reboot -s}
	end
end

desc "Test Avery with Bochs"
task :bochs do
	execute *%w{bin\mcopy -D o -D O -i emu/grubdisk.img@@1M build/kernel.elf ::kernel.elf}
	
	Dir.chdir('emu/') do
		puts "Running Bochs..."
		execute 'bochs', '-q', '-f', 'bochs/bochsrc.bxrc'
	end
end

task :run => [:build, :run]

task :default => :build
