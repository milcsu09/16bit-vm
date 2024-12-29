
attach "tests/stdlib.asm"

mov r5 .message
call .mlen

halt

.message
  define byte "Hello, world!" 0

