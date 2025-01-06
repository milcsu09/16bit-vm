
attach "tests/sdl.asm"

loop:
  mov (FLAG_ADDRESS_EVENT) 1
  mov (FLAG_ADDRESS_CLEAR) 1

  mov   r5 10
  mov   r6 10
  mov w r7 0xFFF
  mov w r8 message1
  call renderer_draw_text

  mov   r5 00
  mov   r6 40
  mov w r7 0xFFF
  mov w r8 message2
  call renderer_draw_text

  mov (FLAG_ADDRESS_DISPLAY) 1
  jmp loop

  halt

message1:
  def "HELLO WORLD" 0

message2:
  def "TEST ASD 123 123" 0

