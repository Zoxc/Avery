#include "syscalls.hpp"
#include "segments.hpp"
#include "interrupts.hpp"
#include "../thread.hpp"
#include "cpu.hpp"

namespace Syscalls
{
	const size_t star = 0xC0000081;
	const size_t lstar = 0xC0000082;
	const size_t cstar = 0xC0000083;
	const size_t sf_mask = 0xC0000084;

	__attribute__((naked)) void syscall_enter();
	__attribute__((noreturn)) void syscall_return();

	extern "C" void syscall(uint64_t rsp, char c, uint64_t, uint64_t rip)
	{
		Thread *thread = CPU::current->thread;

		thread->registers.rsp = rsp;
		thread->registers.rip = rip;

		console.c(c);
	}

	void syscall_enter()
	{
		asm volatile ("swapgs\n"
			"mov %%rsp, %%rdi\n"
			"mov %%gs:%0, %%rsp\n"
			"sti\n"
			"push %%rdi\n" // Contains RSP
			"push %%rcx\n" // Contains RIP
			"call syscall\n"
			"mov %1, %%r11d\n"
			"cli\n"
			"pop %%rcx\n"
			"pop %%rsp\n"
			"swapgs\n"
			"sysretq"
			:: "m"(CPU::current->stack_end), "i"(Arch::Registers::rflags_default));

		__builtin_unreachable();
	}

	void syscall_return()
	{
		Thread *thread = CPU::current->thread;

		asm volatile ("cli\n"
			"mov %0, %%rsp\n"
			"mov %1, %%r11\n"
			"swapgs\n"
			"sysretq" :: "r"(thread->registers.rsp), "r"(thread->registers.rflags), "c"(thread->registers.rip));

		__builtin_unreachable();
	}

	void initialize()
	{
		Arch::write_msr(Arch::efer, Arch::read_msr(Arch::efer) | Arch::efer_bit_syscalls);
		Arch::write_msr(star, (((ptr_t)Segments::user_code_segment) << 48) | ((ptr_t)Segments::code_segment << 32));
		Arch::write_msr(lstar, (ptr_t)&syscall_enter);
		Arch::write_msr(sf_mask, Arch::rflags_bit_interrupt);
	}
};
