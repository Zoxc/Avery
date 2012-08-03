.code32
.align 16
.local stack.32
.comm stack.32, 0x4000

.global entry
entry:
	cli
	movl $(stack.32 + 0x4000), %esp
	pushl %eax
	pushl %ebx
	call setup_long_mode
loop.32:
	hlt
	jmp loop.32

.code64

.global stack
.global bootstrap.64
bootstrap.64:
	# Load data segments
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss
	
	# Load a new higher-half stack
	movabs $(stack + 0x8000), %rsp

	# Call the higher-half entry
	mov %rcx, %rdi
	movabs $boot_entry, %rax
	call *%rax

loop.64:
	hlt
	jmp loop.64
