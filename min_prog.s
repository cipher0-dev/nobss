.global _start

_start:
  movq $60, %rax /* exit syscall number */
  movq $20, %rdi /* exit status */
  syscall
