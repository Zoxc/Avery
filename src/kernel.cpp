#include "multiboot.hpp"
#include "lib.hpp"
#include "x86_64/arch.hpp"
#include "x86_64/console.hpp"

extern "C" void kernel(const multiboot_t &info)
{
	Runtime::initialize();
	
	console.clear().s("Welcome to long mode!").endl();
	
	console.s("Multiboot information at ").x(&info).lb();
	
	Arch::initialize(info);
	
	if(!(info.flags & MULTIBOOT_FLAG_ELF))
		console.panic().s("No ELF information passed!").endl();
	
	console.s("elf header at ").x(info.addr).s(" - ").x(info.size).endl();
	
	asm volatile("int3");
}
