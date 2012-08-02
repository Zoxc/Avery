#include "x86_64/arch.hpp"
#include "lib.hpp"
#include "console.hpp"

void kernel()
{
	console.s("Welcome to Avery!").endl();
	
	Arch::initialize();

	//Arch::enable_interrupts();

	while(true)
		Arch::halt();
}
