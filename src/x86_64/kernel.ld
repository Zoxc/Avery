OUTPUT_FORMAT(elf32-i386)
ENTRY(bootstrap.32)
SECTIONS
{
	. = 0x100000;
	
	.multiboot : { *(.multiboot) }
	
	.bootstrap :
	{
		cache/src/x86_64/bootstrap/*(.text .rodata .data .bss)
	}
	
	. = ALIGN(4096);
	. += 0xFFFFFFFF80000000;
	
	kernel_start = .;
	
	.text ALIGN(4096) : AT(ADDR(.text) - 0xFFFFFFFF80000000) {
		*(.text)
		*(.gnu.linkonce.t*)
	}
	
	.rodata ALIGN(4096) : AT(ADDR(.rodata) - 0xFFFFFFFF80000000)
	{
		ctors_start = .;
		*(.ctor*)
		ctors_end = .;
		
		*(.rodata)
		*(.gnu.linkonce.r*)
	}
	
	.data ALIGN(4096) : AT(ADDR(.data) - 0xFFFFFFFF80000000)
	{
		*(.data)
		*(.gnu.linkonce.d*)
	}
	
	.bss ALIGN(4096) : AT(ADDR(.bss) - 0xFFFFFFFF80000000)
	{
		*(.bss)
		*(.gnu.linkonce.b*)
	}
	
	kernel_end = .;
}
