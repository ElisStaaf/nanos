
.global x86_lgdt
x86_lgdt:
    lgdt (%rdi)
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    popq %rdi
    movq $0x08, %rax
    pushq %rax
    pushq %rdi
    lretq

.global x86_ltr
x86_ltr:
    ltr %di
    retq

.global x86_load_pml4
x86_load_pml4:
	movq $0x000ffffffffff000, %rax
	andq %rax, %rdi
	movq %rdi, %cr3
	retq

.global x86_enable_sse
x86_enable_sse:
    fninit
    fldcw (fcw)
    movq %cr0, %rax
    andw $0xfffb, %ax
    orw $0x2, %ax
    movq %rax, %cr0
    movq %cr4, %rax
    orw $3<<9, %ax
    movq %rax, %cr4
    retq
fcw: .word 0x037F

