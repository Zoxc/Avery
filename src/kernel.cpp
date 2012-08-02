#include "arch.hpp"
#include "lib.hpp"
#include "console.hpp"
#include "params.hpp"
#include "physical_mem_init.hpp"
#include "physical_mem.hpp"

void kernel()
{
	console.s("Welcome to Avery!").endl();

	Arch::initialize_basic();

	Memory::Initial::initialize_physical();

	Arch::initialize_memory();

	Memory::Physical::initialize();

	Arch::initialize();

	//Arch::enable_interrupts();

	while(true)
		Arch::halt();
}
