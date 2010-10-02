.code32
#.local stack
.global stack
.comm stack, 8192

.global bootstrap.32
bootstrap.32:
	cli
	movl $stack + 8192, %esp
	pushl %ebx
	call setup_long_mode
loop.32:
	hlt
	jmp loop.32

.code64

.global bootstrap.64
bootstrap.64:
	# Reset the stack
	movabs $stack + 8192, %rsp
	
	# Zero the upper bits of %rcx
	andq $-1, %rcx
	
	# Load data segments
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	
	# Call kernel
	push %rcx
	call kernel
loop.64:
	hlt
	jmp loop.64
