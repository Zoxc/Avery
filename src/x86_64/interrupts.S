.intel_syntax noprefix
.global spurious_irq
spurious_irq:
	iretq

.global interrupt_handlers

.global isr_handler
isr_handler:
	push rax
	push rcx
	push rdi
	push r8
	push r9
	push r10
	push r11

	mov ax, ds
	push rax

	mov ax, 0x10
	mov ds, ax
	
	mov rdi, rsp
	lea rcx, interrupt_handlers
	call [rcx + 8 * rsi]
	
	pop rax
	mov ds, ax

	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rcx
	pop rax

	pop rsi
	pop rdx

	iretq
