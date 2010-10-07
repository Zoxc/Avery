OUTPUT_FORMAT(elf64-x86-64)
ENTRY(bootstrap.32)
SECTIONS
{
	. = 0x100000;
	
	kernel_start = .;
	
	.multiboot : { *(.multiboot) }
	
	.text : {
		*(.text)
		*(.gnu.linkonce.t*)
	}
	
	.rodata ALIGN(4096) :
	{
		ctors_start = .;
		*(.ctor*)
		ctors_end = .;
		
		*(.rodata)
		*(.gnu.linkonce.r*)
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
	
	kernel_end = .;
	
	. += 0xFFFFFF0000000000;
}
