
mov r1 5
call .factorial
mov @.result_a ac

mov r1 8
call .factorial
mov @.result_b ac

mov r1 @.result_a
mov r2 @.result_b

halt

.factorial
  mov r2 1

.factorial-loop
  mul r2 r2 r1
  sub r1 r1 1

  cmp r1 1
  jgt .factorial-loop

  mov ac r2
  ret

.result_a
  dw 0

.result_b
  dw 0

