OUTPUT_FORMAT(elf64-x86-64)
ENTRY(entry)

high_start = 0xFFFFFFFF80000000;
page_size = 0x1000;

SECTIONS
{
% if multiboot
	. = 0x100000;
	
	.multiboot ALIGN(4) : { *(.multiboot) }
	
	.bootstrap ALIGN(4) :
	{
		build/src/x86_64/multiboot/bootstrap/*(*)
		build/kernel-bootstrap.o(*)
	}
% end
	
	. = ALIGN(page_size);
	low_end = .;
	high_offset = high_start - low_end;
	
	. = high_start;
	
	kernel_start = .;
	
	.text : AT(ADDR(.text) - high_offset)
	{
		*(.text)
		*(.gnu.linkonce.t*)
	}
	
	. = ALIGN(page_size);
	rodata_start = .;
	
	.rodata : AT(ADDR(.rodata) - high_offset)
	{
		ctors_start = .;
		*(.ctor*)
		ctors_end = .;
		
		*(.rodata)
		*(.gnu.linkonce.r*)
		
		raw_bitmap_font = .;
		*(.font)
	}
	
	. = ALIGN(page_size);
	data_start = .;
	
	.data : AT(ADDR(.data) - high_offset)
	{
		*(.data)
		*(.gnu.linkonce.d*)
	}
	
	.bss ALIGN(page_size) : AT(ADDR(.bss) - high_offset)
	{
		*(.bss)
		*(.gnu.linkonce.b*)
	}
	
	. = ALIGN(page_size);
	kernel_end = .;
	kernel_size = (kernel_end - kernel_start) / page_size;
}
