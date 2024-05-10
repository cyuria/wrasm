
.globl _start

.section .text
_start:
    li a0, 1
    la a1, greeting
    li a2, 6
    addi a7, zero, 64 // 64 => write syscall
    ecall

    nop
    li a0, 1
    neg a0, a0
    addi a0, a0, 1
    li a7, 93
    ecall

.section .data
greeting: .ascii "hello\n"

