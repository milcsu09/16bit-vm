
attach "std/stdlib.asm"

mov r5 .message
call .printm

halt

.message
  define byte "Hello, world!" 0

