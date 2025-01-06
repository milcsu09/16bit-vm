
attach "tests/sdl.asm"

loop:
  mov (FLAG_ADDRESS_EVENT) 1
  mov (FLAG_ADDRESS_CLEAR) 1

  mov   r5 (px)
  mov   r6 (py)
  mov w r7 0xAFA
  mov w r8 message1
  call renderer_draw_text

  call move

  mov (FLAG_ADDRESS_DISPLAY) 1
  jmp loop

  halt

move:
  pusha

  mov r1 (px)
  mov r2 (py)

  mov r5 KEY_A
  call keyboard_is_down

  cmp ac 0
  jeq move_skip_a

  sub r1 r1 1

move_skip_a:
  mov r5 KEY_D
  call keyboard_is_down

  cmp ac 0
  jeq move_skip_d

  add r1 r1 1

move_skip_d:
  mov r5 KEY_W
  call keyboard_is_down

  cmp ac 0
  jeq move_skip_w

  sub r2 r2 1

move_skip_w:
  mov r5 KEY_S
  call keyboard_is_down

  cmp ac 0
  jeq move_skip_s

  add r2 r2 1

move_skip_s:
  mov (px) r1
  mov (py) r2

  popa
  ret

message1: def "ABC 123" 0

px: def 25
py: def 25

