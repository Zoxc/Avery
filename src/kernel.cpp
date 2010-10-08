#include "multiboot.hpp"
#include "lib.hpp"
#include "x86_64/arch.hpp"
#include "x86_64/console.hpp"

extern "C" void kernel(const multiboot_t &info)
{
	Runtime::initialize();
	
	console.clear().s("Welcome to long mode!").endl();
	
	Arch::initialize(info);
	
	console.s("Kernel is bored. Formating C:\\... (0%)").endl();
	
	Arch::panic();
}
