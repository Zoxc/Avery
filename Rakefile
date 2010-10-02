require_relative '../reno-0.2/reno'

include Reno
include Reno::Toolchain

output = 'build/kernel.elf'
package = Package.new do
	# name and version
	name 'Avery'
	version '0.0.0'
	
	# setup toolchains
	set Architecture, 'x86_64'
	
	clang = true
	
	set Toolchain::LLVM::Target, 'x86_64-unknown-linux-gnu'
	use Toolchain::LLVM if clang
	
	set Toolchain::GNU::Prefix, 'x86_64-elf-'
	set Toolchain::GNU::Linker::Script, 'src/x86_64/kernel.ld'
	set Toolchain::GNU::Linker::PageSize, 0x1000
	use Toolchain::GNU::Assembler
	use Toolchain::GNU::Linker
	use Toolchain::GNU::Compiler unless clang
	use Toolchain::GNU::Compiler::Preprocessor unless clang
	
	# languages
	use Assembly::WithCPP
	c = use Languages::C
	c.std 'c99'
	
	# files
	boot = collect('src/x86_64/bootstrap/*') do
		set Architecture, 'x86'
		set Toolchain::LLVM::Target, 'x86-unknown-linux-gnu'
	end
	
	# convert to an object file to preserve settings
	# boot = boot.convert(ObjectFile)
	
	files = boot & collect('src/x86_64/*.c') # merge collection, prefer nodes in at the left side
	
	# convert all files to assembly for debug purposes
	files = files.convert(Assembly)
	
	files.merge(Executable).name(output, false)
	Builder.execute 'bin/mbchk', output
end

desc "Build Avery"
task :build do
	package.run
end

desc "Test Avery with QEMU"
task :test => :build do
	Dir.chdir('test/qemu/') do
		Builder.execute *%w{qemu-system-x86_64.exe -m 128 -kernel ./../../build/kernel.elf}
	end
end

desc "Test Avery with Bochs"
task :bochs => :build do
	Builder.execute 'bin/inject',  'test/grub/grub.img', 'test/test.img', output, 'system\\kernel.elf'
	Dir.chdir('test') do
		Builder.execute 'bochs', '-q'
	end
end
task :default => :build