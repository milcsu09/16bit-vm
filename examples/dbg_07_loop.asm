entry:
  # Do nothing 3 times!

  mov r1 3

loop_start:
  cmp r1 0

  jeq loop_end

  sub r1 r1 1

  jmp loop_start

loop_end:
  halt

