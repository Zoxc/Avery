.align 16
.global stack
.comm stack, 0x4000

.global entry
entry:
	# Load a new higher-half stack
	movabs $(stack + 0x4000), %rsp
	
	movabs $kernel, %rax
	call *%rax

loop:
	hlt
	jmp loop
