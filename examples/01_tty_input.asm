
attach "asm/std.asm"

mov r5 s_enter_a_number
call tty_writes

mov r5 buffer1
mov r6 8
call tty_reads

mov r5 s_enter_a_number
call tty_writes

mov r5 buffer2
mov r6 8
call tty_reads

mov r5 buffer1
call std_strtoi

mov r1 ac

mov r5 buffer2
call std_strtoi

mov r2 ac

mov r5 s_add
call tty_writes

add ac r1 r2
print ac

mov r5 s_sub
call tty_writes

sub ac r1 r2
print ac

mov r5 s_mul
call tty_writes

mul ac r1 r2
print ac

mov r5 s_div
call tty_writes

div ac r1 r2
print ac

halt

s_enter_a_number: defb "Enter a number: \0"

s_add: defb "Sum: \0"
s_sub: defb "Difference: \0"
s_mul: defb "Product: \0"
s_div: defb "Quotient: \0"

buffer1: resb 8
buffer2: resb 8

