.intel_syntax noprefix
.align 0x1000
.code16

.global ap_bootstrap
ap_bootstrap:
	cli
	jmp 0:reset_cs
reset_cs:

	xor eax, eax
	mov ds, ax
	mov es, ax

set_A20_ap:
	in al, 0x64
	test al, 0x02
	jnz set_A20_ap

	mov al, 0xD1
	out 0x64, al

check_A20_ap:
	in al, 0x64
	test al, 0x02
	jnz check_A20_ap

	mov al, 0xDF
	out 0x60, al

	lgdt [gdt_pointer.32]

	mov eax, cr0
	or al, 1
	mov cr0, eax

	mov ax, 0x10
	mov ss, ax
	mov ds, ax

	jmp 8:start.32

.code32
start.32:
	mov eax, 0xb8000
	movw [eax], 0x7623
	hlt
	jmp start.32

.align 0x10
gdt.32:
.quad 0

.word 0xFFFF
.word 0
.byte 0
.byte 0x9A
.byte 0xCF
.byte 0

.word 0xFFFF
.word 0
.byte 0
.byte 0x92
.byte 0xCF
.byte 0

.align 0x10
gdt_pointer.32:
.word 23
.long gdt.32
