
attach "asm/std.asm"

entry:
  mov r5 message
  call tty_writes

  halt

message: defb "Hello, world!\n" 0

