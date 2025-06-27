
attach "asm/std.asm"

mov w r5 s
mov w r6 0xDEAD
call std_strcpy

halt

s: def "Hello, world!\n\0"

