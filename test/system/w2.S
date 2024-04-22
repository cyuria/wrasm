
.section .text
_start:
  addi a7, x0, 64
  addi a0, x0, 1
  addi a1, x0, -4
  addi a2, x0, 16
  ecall

  addi a7, x0, 93
  addi a0, x0, 0
  ecall

.section .data
greeting: .asciz "hello\n"

