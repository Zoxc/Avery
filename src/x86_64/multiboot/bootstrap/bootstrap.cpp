#include "common.hpp"
#include "multiboot.hpp"
#include "console.hpp"

#define MULTIBOOT_HEADER_FLAGS MULTIBOOT_HEADER_FLAG_PAGE_ALIGN | MULTIBOOT_HEADER_FLAG_MEMORY_INFO

extern const struct multiboot_header header;

const struct multiboot_header header __attribute__ ((section (".multiboot"))) = {MULTIBOOT_HEADER_MAGIC, MULTIBOOT_HEADER_FLAGS, MULTIBOOT_CHECKSUM(MULTIBOOT_HEADER_FLAGS)};

#define MEMORY_PAGE_ALIGN __attribute__((aligned(0x1000)));

typedef uint64_t table_t[512];
	
static table_t pdpt_low MEMORY_PAGE_ALIGN;
static table_t pdt_low MEMORY_PAGE_ALIGN;
static table_t pt_low MEMORY_PAGE_ALIGN;

static table_t pml4t MEMORY_PAGE_ALIGN;
static table_t pdpt MEMORY_PAGE_ALIGN;
static table_t pdt MEMORY_PAGE_ALIGN;
static table_t pt MEMORY_PAGE_ALIGN;

struct descriptor
{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} __attribute__((packed));

static const struct descriptor gdt[3] = {
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0b10011000, 0b00100000, 0}, // 64-bit code
	{0xFFFF, 0, 0, 0b10010010, 0b11001111, 0}, // data
};

static struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) gdt64_pointer;

static inline uint64_t offset(const void *pointer)
{
	return (uint32_t)pointer;
}

extern void gdt_flush(uint32_t);

void error(const char *msg)
{
	console_fg(console_light_red);
	console_puts("Error");
	console_fg(console_white);
	console_puts(": ");
	console_puts(msg);
}

struct cpuid_result
{
	uint32_t eax, ebx, ecx, edx;
};

static void cpuid(uint32_t input, struct cpuid_result *result)
{
	asm ("cpuid" : "=a"(result->eax), "=b"(result->ebx), "=c"(result->ecx), "=d"(result->edx) : "a"(input));
}

extern void *low_end;
extern void *kernel_size;

extern "C" void setup_long_mode(void *multiboot, uint32_t magic)
{
	console_fg(console_light_gray);
	console_cls();
	console_puts("Booting ");
	console_fg(console_light_green);
	console_puts("Avery");
	console_fg(console_light_gray);
	console_puts("...\n\n");

	if(magic != MULTIBOOT_MAGIC)
	{
		error("This kernel requires a multiboot compatible loader!");
		return;
	}
	
	if((size_t)multiboot + 0x1000 > 0x200000)
	{
		error("Multiboot structure loaded too high in memory");
		return;
	}

	// setup the gdt pointer
	gdt64_pointer.limit = sizeof(gdt) - 1;
	gdt64_pointer.base = offset(gdt);
	
	// setup the higher-half
	pml4t[510] = offset(&pml4t) | 3;
	pml4t[511] = offset(&pdpt) | 3;
	pdpt[510] = offset(&pdt) | 3;
	pdt[0] = offset(&pt) | 3;

	size_t physical = (size_t)&low_end;

	for(size_t i = 0; i < (size_t)&kernel_size; i++, physical += 0x1000)
		pt[i] = physical | 3;

	// setup the lower-half

	pml4t[0] = offset(&pdpt_low) | 3;
	pdpt_low[0] = offset(&pdt_low) | 3;
	pdt_low[0] = offset(&pt_low) | 3;

	// map the first 2 megabytes

	size_t address = 0;

	for(size_t i = 0; i < 512; i++, address += 0x1000)
		pt_low[i] = address | 3;

	struct cpuid_result result;
	
	cpuid(0x80000000, &result);
	
	if(result.eax < 0x80000001)
	{
		error("Long mode is not supported (no extended flags was found)!");
		return;
	}
	
	cpuid(0x80000001, &result);
	
	const unsigned long_mode_flag = 1 << 29;
	
	if(!(result.edx & long_mode_flag))
	{
		error("Long mode is not supported (bit was not set)!");
		return;
	}
	
	console_puts("Entering long mode...");
	
	// load the 64-bit GDT
	asm volatile ("lgdt %0" :: "m"(gdt64_pointer));
	
	// load PML4T into CR3
	asm volatile ("movl %%eax, %%cr3" :: "a"(&pml4t));
	
	// set the long mode bit and nx enable bit
	asm volatile ("rdmsr; orl %0, %%eax; wrmsr" :: "i"((1 << 8) | (1 << 11)), "c"(0xC0000080) : "eax", "edx");
	
	// enable PAE
	asm volatile ("movl %%cr4, %%eax; orl %0, %%eax; movl %%eax, %%cr4" :: "i"(1 << 5) : "eax");
	
	// enable paging and turn on write protect
	asm volatile ("movl %%cr0, %%eax; orl %0, %%eax; movl %%eax, %%cr0" :: "i"((1 << 31) | (1 << 16)) : "eax");
	
	// do a far jump into long mode, pass multiboot information in %ecx
	asm volatile ("ljmp %0, $bootstrap.64" :: "i"(sizeof(struct descriptor) * 1), "c"(multiboot));
}
