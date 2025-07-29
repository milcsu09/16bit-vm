
attach "asm/std.asm"

entry:
  mov r5 65535
  mov r6 buffer
  call std_itoa

  halt

buffer: resb 5

