#include "arch.hpp"
#include "lib.hpp"
#include "console.hpp"
#include "params.hpp"
#include "memory.hpp"
#include "physical_mem_init.hpp"
#include "physical_mem.hpp"
#include "init/init.hpp"

void kernel()
{
	console.s("Welcome to Avery!").endl();

	Arch::initialize_basic();

	Memory::Initial::initialize_physical();

	Arch::initialize_memory();

	Memory::Physical::initialize();

	Memory::initialize();

	Arch::initialize();

	Init::load_modules();

	while(true)
		Arch::halt();
}
