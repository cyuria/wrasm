
.globl _start

.section .text
_start:
  addi a0, zero, 1
  la a1, greeting
  addi a2, zero, 14
  addi a7, zero, 64 // write syscall
  ecall

  addi a7, x0, 93
  addi a0, x0, 0
  ecall

.section .data
greeting: .asciz "Hello, World!\n"

