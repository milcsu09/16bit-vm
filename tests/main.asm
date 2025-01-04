
attach "test"

loop:
  mov (0xfffa) 1

  mov r5 (px)
  mov r6 (py)
  mov w r7 0xFAF
  call draw_point

  mov (0xfffb) 1

  jmp loop
  halt

point_to_index:
  pusha
  
  mov ac 0
  mul ac r6 128
  add ac ac r5
  add w ac ac 0x3000

  popa
  ret

draw_point:
  pusha

  call point_to_index
  mov w (ac) r7

  popa
  ret

px: def 10
py: def 10

