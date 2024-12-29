
attach "std/stdlib.asm"

  mov r1 8

.loop
  cmp byte r1 0
  jeq .end

  mov r5 .buffer
  mov r6 r1
  call .itos

  mov r5 .current_index_is
  call .printm

  mov r5 .buffer
  call .printm

  mov r5 .newline
  call .printm

  mov r5 .buffer
  mov r6 0
  mov r7 ac
  call .mset

  sub byte r1 r1 1
  jmp .loop

.end
  halt

.current_index_is
  define byte "Current index is: " 0

.newline
  define byte '\n' 0

.buffer
  reserve byte 6

