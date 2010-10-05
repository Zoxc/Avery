OUTPUT_FORMAT(elf64-x86-64)
ENTRY(bootstrap.32)
SECTIONS
{
	. = 0x100000;
	
	.multiboot : { *(.multiboot) }
	
	.text : {
		*(.text)
		*(.gnu.linkonce.t*)
	}
	
	.rodata :
	{
		*(.rodata)
		*(.gnu.linkonce.r*)
		
		ctors_start = .;
		*(.ctor*)
		ctors_end = .;
	}
	
	.data :
	{
		*(.data)
		*(.gnu.linkonce.d*)
	}
	
	.bss :
	{
		*(.bss)
		*(.gnu.linkonce.b*)
	}
}
