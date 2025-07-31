entry:
  mov r1 5

  call square

  halt

square:
  # Make r1 it's own square!

  mul r1 r1 r1

  ret

