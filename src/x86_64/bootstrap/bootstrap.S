.code32
.align 16
.local stack
.comm stack, 0x4000

.global bootstrap.32
bootstrap.32:
	cli
	movl $(stack + 0x4000), %esp
	pushl %eax
	pushl %ebx
	call setup_long_mode
loop.32:
	hlt
	jmp loop.32

.code64

.global bootstrap.64
bootstrap.64:
	# Reset the stack
	movabs $(stack + 0x4000), %rsp
	
	# Load data segments
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	
	# Zero the upper bits of %rcx
	#andq $0xFFFFFFFF, %rcx
	
	# Call kernel
	mov %rcx, %rdi
	movabs $kernel, %rax
	call *%rax

loop.64:
	hlt
	jmp loop.64
