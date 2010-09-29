#include "../../multiboot.h"
#include "console.h"
#include "common.h"

BOOTSTRAP_CODE

#define MULTIBOOT_HEADER_FLAGS MULTIBOOT_HEADER_FLAG_PAGE_ALIGN | MULTIBOOT_HEADER_FLAG_MEMORY_INFO

const struct multiboot_header header __attribute__ ((section (".multiboot"))) = {MULTIBOOT_MAGIC, MULTIBOOT_HEADER_FLAGS, MULTIBOOT_CHECKSUM(MULTIBOOT_HEADER_FLAGS)};

static uint64_t page_pml4t[512] __attribute__((aligned(0x1000)));
static uint64_t page_pdpt[512] __attribute__((aligned(0x1000)));
static uint64_t page_pdt[512] __attribute__((aligned(0x1000)));
static uint64_t page_pt[512] __attribute__((aligned(0x1000)));

struct descriptor
{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} __attribute__((packed));

static struct descriptor gdt64[] = {
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0b10011000, 0b00100000, 0}, // 64-bit code
	{0xFFFF, 0, 0, 0b10010000, 0b11001111, 0}, // data
	{0xFFFF, 0, 0, 0b10011010, 0b11001111, 0}, // 32-bit code
};

static struct {
	uint32_t limit;
	uint64_t base;
} __attribute__((packed)) gdt64_pointer;

static inline uint64_t offset(void *pointer)
{
	return (uint32_t)pointer;
}

void setup_long_mode(multiboot_t *multiboot)
{
	console_fg(console_light_gray);
	console_bg(console_black);
	console_cls();
	
	kprint("Booting up...\nMultiboot at 0x%x\n", multiboot);

	// setup the gdt pointer
	gdt64_pointer.limit = sizeof(gdt64) - 1;
	gdt64_pointer.base = offset(&gdt64);

	// load the GDT
	asm volatile ("lgdt %0; movw %1, %%ax; movw %%ax, %%ds; movw %%ax, %%es; movw %%ax, %%fs; movw %%ax, %%gs;  ljmp %2, $setup_long_mode_reload; setup_long_mode_reload:" :: "i"(gdt64_pointer), "i"(sizeof(struct descriptor) * 2), "i"(sizeof(struct descriptor) * 3) : "eax");
	return;

	page_pml4t[0] = offset(&page_pdpt) | 3;
	page_pdpt[0] = offset(&page_pdt) | 3;
	page_pdt[0] = offset(&page_pt) | 3;
	
	kprint("gdt @ 0x%x\npml4t @ 0x%x\npdpt @ 0x%x\npdt @ 0x%x\npt @ 0x%x\n", &gdt64, &page_pml4t, &page_pdpt, &page_pdt, &page_pt);

	// map the first 2 megabytes
	unsigned int i, address = 0;
	for(i = 0; i < 512; i++)
	{
		page_pt[i] = address | 3; // map address and mark it present/writable
		address += 0x1000;
	}

	uint32_t result;

	asm ("cpuid" : "=a"(result) : "a"(0x80000000));
	
	if(result < 0x80000001)
	{
		kprint("Long mode is not supported (no extended flags was found)!");
		return;
	}

	asm ("cpuid" : "=d"(result) : "a"(0x80000001) : "ecx");
	
	const unsigned long_mode_flag = 1 << 29;

	if(!(result & long_mode_flag))
	{
		kprint("Long mode is not supported (bit was not set)!");
		return;
	}

	kprint("Entering long mode...");
	// load PML4T into CR3
	asm volatile ("movl %%eax, %%cr3" :: "a" (&page_pml4t));

	// set the long mode bit
	asm volatile ("rdmsr; orl %0, %%eax; wrmsr" :: "i"(1 << 8), "c"(0xC0000080) : "eax", "edx");
	
	// enable paging and PAE
	asm volatile ("movl %%cr0, %%eax; orl %0, %%eax; movl %%eax, %%cr0" :: "i"(1 << 31 | 1 << 5) : "eax");

	// do a far jump into long mode, pass multiboot information in %ecx
	asm volatile ("ljmp %0, $bootstrap.64" :: "i"(sizeof(struct descriptor) * 1), "c"(multiboot));
	
	__builtin_unreachable();
}
