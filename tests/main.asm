
attach "tests/sdl.asm"

loop:
  mov (FLAG_ADDRESS_EVENT) 1
  mov (FLAG_ADDRESS_CLEAR) 1

  mov   r5 10
  mov   r6 10
  mov w r7 0xFAF
  call renderer_draw_point

  # mov   r5 20
  # mov   r6 20
  # mov w r7 0xAFA
  # call renderer_draw_point

  mov (FLAG_ADDRESS_DISPLAY) 1

  jmp loop
  halt

