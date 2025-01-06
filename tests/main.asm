
attach "tests/sdl.asm"

loop:
  mov (FLAG_ADDRESS_EVENT) 1
  mov (FLAG_ADDRESS_CLEAR) 1

  mov   r5 10
  mov   r6 10
  mov w r7 0xFFF
  mov   r8 'H
  call renderer_draw_font

  mov   r5 18
  mov   r6 10
  mov w r7 0xFFF
  mov   r8 'E
  call renderer_draw_font

  mov   r5 27
  mov   r6 10
  mov w r7 0xFFF
  mov   r8 'L
  call renderer_draw_font

  mov   r5 36
  mov   r6 10
  mov w r7 0xFFF
  mov   r8 'L
  call renderer_draw_font

  mov   r5 45
  mov   r6 10
  mov w r7 0xFFF
  mov   r8 'O
  call renderer_draw_font


end:
  mov (FLAG_ADDRESS_DISPLAY) 1
  jmp loop

  halt

