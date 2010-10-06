#include "multiboot.hpp"
#include "lib.hpp"
#include "x86_64/arch.hpp"
#include "x86_64/console.hpp"

extern "C" void kernel(const multiboot_t &info)
{
	Runtime::initialize();
	
	console.clear().s("Welcome to long mode!").endl();
	console.s("unsigned long is ").x(sizeof(unsigned long)).endl();
	
	Arch::initialize(info);
	
	asm volatile("int3");
}
