
attach "asm/std.asm"

mov w r5 s_enter_a_number
call tty_writes

mov w r5 buffer1
mov r6 8
call tty_reads

mov w r5 s_enter_a_number
call tty_writes

mov w r5 buffer2
mov r6 8
call tty_reads

mov w r5 buffer1
call std_strtoi

mov r1 ac

mov w r5 buffer2
call std_strtoi

mov r2 ac

mov w r5 s_add
call tty_writes

add ac r1 r2
print ac

mov w r5 s_sub
call tty_writes

sub ac r1 r2
print ac

mov w r5 s_mul
call tty_writes

mul ac r1 r2
print ac

mov w r5 s_div
call tty_writes

div ac r1 r2
print ac

halt

s_enter_a_number: def "Enter a number: \0"

s_add: def "Sum: \0"
s_sub: def "Difference: \0"
s_mul: def "Product: \0"
s_div: def "Quotient: \0"

buffer1: res 8
buffer2: res 8

