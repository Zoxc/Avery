OUTPUT_FORMAT(elf64-x86-64)
ENTRY(entry)

high_start = 0xFFFFFFFF80000000;
	
SECTIONS
{

	. = 0x100000;
	
	.multiboot ALIGN(4) : { *(.multiboot) }
	
	.bootstrap :
	{
		build/src/x86_64/multiboot/bootstrap/*(*)
		build/kernel-bootstrap.o(*)
	}
	
	. = ALIGN(4096);
	high_offset = high_start - .;
	
	. = high_start;
	
	kernel_start = .;
	
	.text ALIGN(0x1000) : AT(ADDR(.text) - high_offset)
	{
		*(.text)
		*(.gnu.linkonce.t*)
	}
	
	.rodata ALIGN(0x1000) : AT(ADDR(.rodata) - high_offset)
	{
		ctors_start = .;
		*(.ctor*)
		ctors_end = .;
		
		*(.rodata)
		*(.gnu.linkonce.r*)
		
		raw_bitmap_font = .;
		*(.font)
	}
	
	.data ALIGN(0x1000) : AT(ADDR(.data) - high_offset)
	{
		*(.data)
		*(.gnu.linkonce.d*)
	}
	
	.bss ALIGN(0x1000) : AT(ADDR(.bss) - high_offset)
	{
		*(.bss)
		*(.gnu.linkonce.b*)
	}
	
	kernel_end = .;
}
