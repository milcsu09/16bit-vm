
attach "asm/std.asm"

mov w r5 s1
call tty_write

mov w r5 s2
call tty_write

halt

s1: def "Hello " 0
s2: def "world!" 10 0

