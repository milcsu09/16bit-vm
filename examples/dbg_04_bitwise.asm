entry:
  # Swap r1 and r2 with XOR!

  mov r1 10
  mov r2 20

  xor r1 r1 r2
  xor r2 r1 r2
  xor r1 r1 r2

  halt

