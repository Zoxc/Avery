#include "../lib.hpp"
#include "debug.hpp"
#include "console.hpp"

namespace Debug
{
	struct SectionHeader
	{
		uint32_t name;
		uint32_t type;
		uint32_t flags;
		uint32_t addr;
		uint32_t offset;
		uint32_t size;
		uint32_t link;
		uint32_t info;
		uint32_t addralign;
		uint32_t entsize;
	} __attribute__((packed));
}

const Debug::Symbol *Debug::symbols;
const Debug::Symbol *Debug::symbols_end;

const char *Debug::symbol_names;
const char *Debug::symbol_names_end;

void Debug::initialize(const multiboot_t &info)
{
	if(!(info.flags & MULTIBOOT_FLAG_ELF))
	{
		console.s("No ELF information passed. Symbol information disabled.").endl();
		return;
	}
	
	SectionHeader *headers = (SectionHeader *)info.addr;
	
	uint32_t section_names = headers[info.shndx].addr;
	
	for(size_t i = 0; i < info.num; ++i)
	{
		const char *name = (const char *)(section_names + headers[i].name);
		
		if(strcmp(name, ".strtab") == 0)
		{
			symbol_names = (const char *)headers[i].addr;
			symbol_names_end = (const char *)(headers[i].addr + headers[i].size);
		}
		
		if(strcmp(name, ".symtab") == 0)
		{
			symbols = (const struct Symbol *)headers[i].addr;
			symbols_end = (const struct Symbol *)(headers[i].addr + headers[i].size);
		}
	}
}