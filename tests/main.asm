
attach "tests/fixed.asm"
attach "tests/sdl.asm"

loop:
  mov (FLAG_ADDRESS_EVENT) 1
  mov (FLAG_ADDRESS_CLEAR) 1

  mov w r5 15
  mov w r6 15
  mov w r7 125
  mov w r8 125
  push w 0x111
  call renderer_draw_rect

  add sp sp 2

  mov w r5 10
  mov w r6 10
  mov w r7 110
  mov w r8 110
  push w 0x333
  call renderer_draw_rect

  add sp sp 2

  # mov w (0x100) 1

  ##################

  mov w r5 10
  mov w r6 10
  mov w r7 20
  mov w r8 20
  push w 0xF00
  call renderer_draw_rect

  add sp sp 2

  mov w r5 30
  mov w r6 10
  mov w r7 40
  mov w r8 20
  push w 0x0F0
  call renderer_draw_rect

  add sp sp 2

  mov w r5 50
  mov w r6 10
  mov w r7 60
  mov w r8 20
  push w 0x00F
  call renderer_draw_rect

  add sp sp 2

  mov w r5 70
  mov w r6 10
  mov w r7 80
  mov w r8 20
  push w 0xFAF
  call renderer_draw_rect

  add sp sp 2

  ##################

  mov w r5 10
  mov w r6 40
  mov w r7 128
  mov w r8 128
  push w 0x300
  call renderer_draw_rect

  add sp sp 2

  mov w r5 30
  mov w r6 40
  mov w r7 128
  mov w r8 108
  push w 0x030
  call renderer_draw_rect

  add sp sp 2

  mov w r5 50
  mov w r6 40
  mov w r7 128
  mov w r8 88
  push w 0x003
  call renderer_draw_rect

  add sp sp 2

  mov w r5 70
  mov w r6 40
  mov w r7 128
  mov w r8 68
  push w 0x313
  call renderer_draw_rect

  add sp sp 2

  ##################

  mov w r5 10
  mov w r6 50
  mov w r7 0xFF0
  mov w r8 message
  call renderer_draw_text

  mov w r5 15
  mov w r6 55
  mov w r7 0xAAA
  mov w r8 message
  call renderer_draw_text

  mov w r5 20
  mov w r6 60
  mov w r7 0xBBB
  mov w r8 message
  call renderer_draw_text

  mov w r5 25
  mov w r6 65
  mov w r7 0xCCC
  mov w r8 message
  call renderer_draw_text

  mov w r5 30
  mov w r6 70
  mov w r7 0xDDD
  mov w r8 message
  call renderer_draw_text

  mov w r5 35
  mov w r6 75
  mov w r7 0xEEE
  mov w r8 message
  call renderer_draw_text

  mov w r5 40
  mov w r6 80
  mov w r7 0xFFF
  mov w r8 message
  call renderer_draw_text

  ##################

  mov w r5 (px)
  mov w r6 (py)
  mov w r7 0xF00

  mov r1 (dx)
  cmp r1 0
  jeq dir_right

  add r5 r5 1
  jmp dir_end

dir_right:
  sub r5 r5 1

dir_end:
  cmp r5 10
  jne move_cont

  mov r1 1

move_cont:
  cmp r5 110
  jne move_end

  mov r1 0

move_end:
  call renderer_draw_point

  mov (px) r5
  mov (dx) r1

  ##################

  mov (FLAG_ADDRESS_DISPLAY) 1
  jmp loop

  halt

message: def "HELLO WORLD\0"

px: def w 30
py: def w 30

dx: def 0
# dy: def w 1

