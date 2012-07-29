#include "x86_64/arch.hpp"
#include "lib.hpp"
#include "console.hpp"

void kernel()
{
	Runtime::initialize();
	
	console.initialize();
	
	console.clear().s("Welcome to Avery!").endl();
	
	Arch::initialize();
	
	Arch::panic();
}
