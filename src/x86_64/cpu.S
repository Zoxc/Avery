.intel_syntax noprefix
.align 0x1000
.code16

.global ap_bootstrap
ap_bootstrap:
	cli
	xor eax, eax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax

	jmp 0:reset_cs
reset_cs:
	mov ax, 0xB800
	mov ds, ax
	movw [ds:0], 0x7623
	jmp reset_cs

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

	jmp 8:start.32

.code32
start.32:
	mov eax, 0xb8000
	movw [eax], 0x7623
	hlt
	jmp start.32

gdt.32:
.quad 0
.quad 0xCF9A000000FFFF
.quad 0xCF92000000FFFF

gdt_pointer.32:
.word 23
.long gdt.32
