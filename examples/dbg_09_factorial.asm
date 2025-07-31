entry:
  mov r5 5
  call factorial

  halt

factorial: ; (r5 n) -> ac
  pusha
  mov ac 1

factorial_loop:
  cmp r5 1
  jle factorial_end

  mul ac ac r5
  sub r5 r5 1

  jmp factorial_loop

factorial_end:
  popa
  ret

