
mov r1 0
mov r2 1

mov r3 0

fibonacci:
  cmp r3 3
  jge fibonacci_end

  print r1

  mov ac r1
  add r1 r1 r2
  mov r2 ac

  add r3 r3 1
  jmp fibonacci

fibonacci_end:
  halt

