
attach "tests/fixed.asm"
attach "tests/sdl.asm"

loop:
  mov (FLAG_ADDRESS_EVENT) 1
  mov (FLAG_ADDRESS_CLEAR) 1

  mov w r5 10
  mov w r6 10
  mov w r7 126
  mov w r8 126
  push w 0xFAF
  call renderer_draw_rect

  add sp sp 2

  mov (FLAG_ADDRESS_DISPLAY) 1
  jmp loop

  halt

px: def w 0
py: def w 0

