
attach "asm/std.asm"

mov w r5 buffer
mov r6 16
call tty_reads

mov w r5 buffer
call tty_writes

tty_write '\n

halt

buffer: res 16

