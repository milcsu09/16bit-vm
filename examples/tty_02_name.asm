
attach "asm/std.asm"

entry:
  mov r5 s_whats_your_name
  call tty_writes

  mov r5 b_name
  mov r6 32
  call tty_reads

  mov r5 s_hello
  call tty_writes

  mov r5 b_name
  call tty_writes

  tty_write '!
  tty_write '\n

  halt

s_whats_your_name: defb "What's your name? \0"
s_hello: defb "Hello, \0"

b_name: resb 32

