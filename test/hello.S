#include <asm-generic/unistd.h>

.section .rodata

greeting: .asciz "Hello, World!\n"
.equ greetlen, . - greeting

.text
.global _start
_start:
  li a7, __NR_write
  li a0, 1
  la a1, greeting
  li a2, greetlen
  ecall

  li a7, __NR_exit
  li a0, 0
  ecall

1: j 1b

