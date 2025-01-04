
attach "tests/sdl.asm"

loop:
  mov (FLAG_ADDRESS_EVENT) 1
  mov (FLAG_ADDRESS_CLEAR) 1

  mov   r5 KEY_SPACE
  call keyboard_is_down

  cmp ac 0
  jeq end

  mov   r5 10
  mov   r6 20
  mov w r7 0xFAF
  call renderer_draw_point

end:
  mov (FLAG_ADDRESS_DISPLAY) 1

  jmp loop
  halt

