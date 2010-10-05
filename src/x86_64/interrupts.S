# Macro for interrupt vectors without an error code
#define no_error_code(num) \
  .global isr ## num ;\
  isr ## num: ;\
    cli ;\
    pushq $0 ;\
    pushq $ ## num   ;\
    jmp isr_stub
	
# Macro for interrupt vectors with an error code
#define error_code(num) \
  .global isr ## num ;\
  isr ## num: ;\
    cli ;\
    pushq $ ## num   ;\
    jmp isr_stub

# Macro for IRQs
#define irq(num) \
  .global irq ## num ;\
  irq ## num: ;\
    cli ;\
    pushq 0   ;\
    pushq $(num + 32) ;\
    jmp irq_stub

no_error_code(0)
no_error_code(1)
no_error_code(2)
no_error_code(3)
no_error_code(4)
no_error_code(5)
no_error_code(6)
no_error_code(7)
error_code(8)
no_error_code(9)
error_code(10)
error_code(11)
error_code(12)
error_code(13)
error_code(14)
no_error_code(15)
no_error_code(16)
no_error_code(17)
no_error_code(18)
no_error_code(19)
no_error_code(20)
no_error_code(21)
no_error_code(22)
no_error_code(23)
no_error_code(24)
no_error_code(25)
no_error_code(26)
no_error_code(27)
no_error_code(28)
no_error_code(29)
no_error_code(30)
no_error_code(31)
no_error_code(128)
irq(0)
irq(1)
irq(2)
irq(3)
irq(4)
irq(5)
irq(6)
irq(7)
irq(8)
irq(9)
irq(10)
irq(11)
irq(12)
irq(13)
irq(14)
irq(15)

isr_stub:
	# Push all registers
	push %rax
	push %rcx
	push %rdx
	push %rbx
	push %rbp
	push %rsi
	push %rdi
	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15

    mov %ds, %ax               # Lower 16-bits of eax = ds.
    push %rax                 # save the data segment descriptor

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es 
    mov %ax, %fs 
    mov %ax, %gs 
	
	mov %rsp, %rdi
    call isr_handler
	
    pop %rax        # reload the original data segment descriptor
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
	
	# Pop all registers
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rdi
	pop %rsi
	pop %rbp
	pop %rbx
	pop %rdx
	pop %rcx
	pop %rax

    add 16, %rsp     # Cleans up the pushed error code and pushed ISR number
    sti
    iret           # pops 5 things at once: CS, RIP, RFLAGS, SS, and RSP

irq_stub:
	# Push all registers
	push %rax
	push %rcx
	push %rdx
	push %rbx
	push %rbp
	push %rsi
	push %rdi
	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15

    mov %ds, %ax               # Lower 16-bits of eax = ds.
    push %rax                 # save the data segment descriptor

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es 
    mov %ax, %fs 
    mov %ax, %gs 

	mov %rsp, %rdi
    call irq_handler

    pop %rax        # reload the original data segment descriptor
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
	
	# Pop all registers
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rdi
	pop %rsi
	pop %rbp
	pop %rbx
	pop %rdx
	pop %rcx
	pop %rax

    add 16, %rsp     # Cleans up the pushed error code and pushed ISR number
    sti
    iret           # pops 5 things at once: CS, RIP, RFLAGS, SS, and RSP
