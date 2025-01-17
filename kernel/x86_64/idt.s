
.global x86_load_idt
x86_load_idt:
  lidtq (%rdi)
  retq

.macro pushaq_norax
  pushq %rbx
  pushq %rcx
  pushq %rdx
  pushq %rbp
  pushq %rdi
  pushq %rsi
  pushq %r8
  pushq %r9
  pushq %r10
  pushq %r11
  pushq %r12
  pushq %r13
  pushq %r14
  pushq %r15
.endm

.macro popaq_norax
  popq %r15
  popq %r14
  popq %r13
  popq %r12
  popq %r11
  popq %r10
  popq %r9
  popq %r8
  popq %rsi
  popq %rdi
  popq %rbp
  popq %rdx
  popq %rcx
  popq %rbx
.endm

.extern x86_int_handler
common_isr:
  pushaq_norax /* rax is already pushed by the isrs themselves */
  movq %rsp, %rdi
  movq %rax, %rsi
  movq %rsp, %rbx
  callq x86_int_handler
  movq %rbx, %rsp
  popaq_norax
  cmpq $0, %rax
  je .nomove
  movq %rax, 16(%rsp) /* sets return address if not zero ; addr err rax ; +16 +8 +0 */
.nomove:
  popq %rax
  addq $8, %rsp /* error code */
  iretq

.macro isr_code id
.global x86_isr\id
x86_isr\id:
  pushq %rax
  movq $\id, %rax
  jmp common_isr
.endm

.macro isr_nocode id
.global x86_isr\id
x86_isr\id:
  pushq $0
  pushq %rax
  movq $\id, %rax
  jmp common_isr
.endm

isr_nocode 0
isr_nocode 1
isr_nocode 2
isr_nocode 3
isr_nocode 4
isr_nocode 5
isr_nocode 6
isr_nocode 7
isr_code 8
isr_nocode 9
isr_code 10
isr_code 11
isr_code 12
isr_code 13
isr_code 14
isr_nocode 15
isr_nocode 16
isr_code 17
isr_nocode 18
isr_nocode 19
isr_nocode 20
isr_code 21
isr_nocode 22
isr_nocode 23
isr_nocode 24
isr_nocode 25
isr_nocode 26
isr_nocode 27
isr_nocode 28
isr_code 29
isr_code 30
isr_nocode 31
