
attach "asm/std.asm"

mov r5 420
mov r6 buffer
call std_itoa

halt

buffer: resb 5

