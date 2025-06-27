
attach "tests/fixed.asm"

# EXPECTED : 15.968719422671311 (Python 3.11.2)
# GOT      : 15.96484375
#          ~  0.0039 (~1 ULP)
mov w r5 255.0
call fsqrt

mov w r1 1

halt

fsqrt:
  pusha

  # i = 0
  mov r1 0

  # guess = x
  mov r2 r5

fsqrt_loop:
  cmp r1 8
  jge fsqrt_end

  # 'r5' already in-place
  call fset

  # x / guess
  push r5
  mov r5 r2
  call fdiv
  pop r5

  # (guess + x / guess) * 0.5
  add ac ac r2
  shr ac ac 1

  # guess = (guess + x / guess) * 0.5
  mov r2 ac

  add r1 r1 1
  jmp fsqrt_loop

fsqrt_end:
  mov ac r2

  popa
  ret

