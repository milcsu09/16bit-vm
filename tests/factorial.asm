
mov r1 5
mov r2 1

.factorial-loop
  mul r2 r2 r1
  sub r1 r1 1

  cmp r1 1
  jgt .factorial-loop

halt

