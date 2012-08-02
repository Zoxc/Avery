.global load_segments
load_segments:
	pushq %rdi
	pushq $new_code
	ljmp *(%rsp)

	pushq $new_code
	pushq %rsi
	pushq $0
	pushq $0
	pushq $0
	pushq $0
	pushq $0
	lret
	
new_code:
	movw %di, %ax
	movw %ax, %ss
	movw %ax, %ds

	ret
