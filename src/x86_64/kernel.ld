OUTPUT_FORMAT(elf64-x86-64)
ENTRY(entry)
SECTIONS
{
	. = 0xFFFFFFFF80000000;
	
	kernel_start = .;
	
	.text ALIGN(0x1000) :
	{
		*(.text)
		*(.gnu.linkonce.t*)
	}
	
	.rodata ALIGN(0x1000) :
	{
		ctors_start = .;
		*(.ctor*)
		ctors_end = .;
		
		*(.rodata)
		*(.gnu.linkonce.r*)
		
		raw_bitmap_font = .;
		*(.font)
	}
	
	.data ALIGN(0x1000) :
	{
		*(.data)
		*(.gnu.linkonce.d*)
	}
	
	.bss ALIGN(0x1000) :
	{
		*(.bss)
		*(.gnu.linkonce.b*)
	}
	
	kernel_end = .;
}
