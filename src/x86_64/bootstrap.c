#include "../multiboot.h"

asm(".code32");

#define MULTIBOOT_HEADER_FLAGS MULTIBOOT_HEADER_FLAG_PAGE_ALIGN | MULTIBOOT_HEADER_FLAG_MEMORY_INFO

const struct multiboot_header header __attribute__ ((section (".multiboot"))) = {MULTIBOOT_MAGIC, MULTIBOOT_HEADER_FLAGS, MULTIBOOT_CHECKSUM(MULTIBOOT_HEADER_FLAGS)};

static uint64_t page_map[512] __attribute__((aligned(0x1000)));
static uint64_t page_dir[512] __attribute__((aligned(0x1000)));
static uint64_t page_tab[512] __attribute__((aligned(0x1000)));

struct descriptor
{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
};

struct gdt_pointer
{
	uint32_t limit;
	uint64_t base;
};

static struct descriptor gdt64[3] = {
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0b10011000, 0b00100000, 0},
	{0, 0, 0, 0b10010000, 0b00000000, 0},
};

static struct gdt_pointer gdt64_pointer = {sizeof(gdt64) - 1, 0};

static char *const vga = (char *const)0xb8000;

static char *position = (char *const)0xb8000;

static void print(const char *str)
{
	if(str == 0)
		return;

	while(*str)
	{
		*position++ = *str++;
		position++;
	}
}

static inline uint64_t offset(void *pointer)
{
	return (uint32_t)pointer;
}

void setup_long_mode(multiboot_t *multiboot)
{
	print("Booting up...");

	// setup the gdt pointer
	gdt64_pointer.base = offset(gdt64);

	page_map[0] = offset(&page_dir) | 3; // set the page directory into the PDPT and mark it present
	page_dir[0] = offset(&page_tab) | 3; // set the page table into the PD and mark it present/writable

	// map the first 2 megabytes
	unsigned int i, address = 0;
	for(i = 0; i < 512; i++)
	{
		page_tab[i] = address | 3; // map address and mark it present/writable
		address = address + 0x1000;
	}

	// enable PAE
 	asm ("movl %%cr4, %%eax; bts $5, %%eax; movl %%eax, %%cr4" ::: "eax");

	// load PML4T into CR3
	asm ("movl %%eax, %%cr3" :: "a" (&page_map));

	uint32_t result;

	asm ("cpuid" : "=a"(result) : "a"(0x80000000));
	
	if(result < 0x80000001)
	{
		print("Long mode is not supported (no extended flags was found)!");
		return;
	}

	asm ("cpuid" : "=d"(result) : "a"(0x80000001) : "ecx");
	
	const unsigned long_mode_flag = 1 << 29;

	if(!(result & long_mode_flag))
	{
		print("Long mode is not supported (bit was not set)!");
		return;
	}

	print("Entering long mode...");

	// set the long mode bit
	asm volatile ("rdmsr; orl %0, %%eax; wrmsr" :: "i"(1 << 8), "c"(0xC0000080));

	// enable paging
	asm volatile ("movl %%cr0, %%eax; orl %0, %%eax; wrmsr" :: "i"(1 << 31), "c"(0xC0000080));

	// load the GDT and do a far jump into long mode, pass multiboot information in %ecx
	asm volatile ("lgdt %0; ljmp %1, $bootstrap.64" :: "i"(gdt64_pointer), "i"(sizeof(struct descriptor) * 1), "c"(multiboot));
}
