#include "arch.hpp"
#include "debug.hpp"
#include "physical_mem.hpp"
#include "physical_mem_init.hpp"
#include "memory.hpp"
#include "../console.hpp"

void Arch::initialize()
{
	initialize_gdt();
	initialize_idt();
	
	Debug::initialize();
	return;
	Memory::Initial::initialize_physical();
	
	for(Memory::Initial::Entry *entry = Memory::Initial::list; entry; entry = entry->get_next())
		console.s("- Hole1 @ ").x(entry).s(" : ").x(entry->next_low).s(" - ").x(entry->get_next()).lb();
	
	Memory::Initial::initialize();
	
	for(Memory::Initial::Entry *entry = Memory::Initial::list; entry; entry = entry->get_next())
		console.s("- Hole2 @ ").x(entry).s(" : ").x(entry->next_low).s(" - ").x(entry->get_next()).lb();
	
	Memory::Physical::initialize();
}

void Arch::panic()
{
	while(1) asm volatile("hlt");
}