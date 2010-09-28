.code32
.local stack
.comm stack, 4096

.global bootstrap.32
bootstrap.32:
	cli
	movl $stack + 4096, %esp
	pushl %ebx
	call setup_long_mode
loop.32:
	hlt
	jmp loop.32

.code64

.global bootstrap.64
bootstrap.64:
	# Zero the upper bits of %rcx
	andq 0xFFFFFFFF, %rcx
	
	# Load data segments
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	
	# Call kernel
	pushq %rcx
	call kernel
loop.64:
	hlt
	jmp loop.64
