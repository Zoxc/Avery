#include "../arch.hpp"
#include "../../process.hpp"
#include "../../thread.hpp"
#include "elf.h"

void Init::enter_usermode(Thread *thread)
{
	asm volatile (
		"cli\n"
		"swapgs\n"
		"mov %%ax, %%ds\n"
		"mov %0, %%rsp\n"
		"iretq" :: "r"(&thread->registers.rip), "a"(Segments::user_data_segment));
}

ptr_t Init::load_module(Process *process, const void *obj, size_t)
{
	uint8_t *buffer = (uint8_t *)obj;

	Elf64_Ehdr *header = (Elf64_Ehdr *)buffer;
	Elf64_Phdr *program_header = (Elf64_Phdr *)(buffer + header->e_phoff);

	assert(strncmp((const char *)header->e_ident, "\x7F" "ELF", 4) == 0, "Module is not of ELF format");
	assert(header->e_type == ET_EXEC, "Module is not executable");
	assert(header->e_machine == EM_X86_64, "Module is not x86_64");

	for(size_t i = 0; i < header->e_phnum; ++i, program_header = (Elf64_Phdr *)((size_t)program_header + header->e_phentsize))
	{
		if(program_header->p_type != PT_LOAD)
			continue;

		ptr_t start = program_header->p_vaddr;
		ptr_t end = start + program_header->p_memsz;

		Memory::assert_page_aligned(start);

		ptr_t aligned_end = align_up(end, Arch::page_size);
		ptr_t aligned_pages = (aligned_end - start) / Arch::page_size;

		User::Block *segment = process->allocator.allocate_at((Memory::VirtualPage *)start, User::Block::Generic, aligned_pages);

		size_t flags = Memory::present_bit | Memory::usermode_bit;

		if(program_header->p_flags & PF_W)
			flags |= Memory::write_bit;

		if(!(program_header->p_flags & PF_X))
			flags |= Memory::nx_bit;

		Memory::map(segment->base, aligned_pages, Memory::rw_data_flags, &process->address_space);

		assert(program_header->p_memsz >= program_header->p_filesz, "Module file size is larger than memory size");

		memcpy(segment->base, buffer + program_header->p_offset, program_header->p_filesz);

		Memory::protect(segment->base, aligned_pages, flags);
	}

	return header->e_entry;
}
