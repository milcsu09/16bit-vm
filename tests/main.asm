
attach "tests/stdlib.asm"

mov r5 0x7000
mov r6 'A'
mov r7 30
call .mset

halt

