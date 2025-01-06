
attach "tests/sdl.asm"

loop:
  mov (FLAG_ADDRESS_EVENT) 1
  mov (FLAG_ADDRESS_CLEAR) 1

  mov   r5 10
  mov   r6 10
  mov w r7 0xFFF
  mov   r8 '0
  call renderer_draw_font

  mov   r5 18
  mov   r6 10
  mov w r7 0xFAF
  mov   r8 '1
  call renderer_draw_font

  mov   r5 27
  mov   r6 10
  mov w r7 0xFFA
  mov   r8 '2
  call renderer_draw_font

  mov   r5 36
  mov   r6 10
  mov w r7 0xAFF
  mov   r8 '3
  call renderer_draw_font

  mov   r5 45
  mov   r6 10
  mov w r7 0xAFA
  mov   r8 '4
  call renderer_draw_font

  mov   r5 54
  mov   r6 10
  mov w r7 0xAAF
  mov   r8 '5
  call renderer_draw_font

  mov   r5 63
  mov   r6 100
  mov w r7 0xFAA
  mov   r8 '6
  call renderer_draw_font

  mov   r5 72
  mov   r6 10
  mov w r7 0xAFF
  mov   r8 '7
  call renderer_draw_font

  mov   r5 81
  mov   r6 10
  mov w r7 0xFFF
  mov   r8 '8
  call renderer_draw_font

  mov   r5 90
  mov   r6 10
  mov w r7 0x666
  mov   r8 '9
  call renderer_draw_font

  mov (FLAG_ADDRESS_DISPLAY) 1

  jmp loop
  halt

