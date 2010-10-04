#include "multiboot.hpp"
#include "lib.hpp"
#include "x86_64/console.hpp"

extern "C" void kernel(multiboot_t *multiboot)
{
	Runtime::initialize();
	
	console.clear().puts("Welcome to long mode!");
}
