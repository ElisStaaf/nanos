section .text.prologue
[extern main]
[global _start]
_start:
    call main
    mov rdi, rax
    mov rax, 0
    int 1

[global sys_funcs]
sys_funcs:
    mov rax, 1
    int 1
    ret
