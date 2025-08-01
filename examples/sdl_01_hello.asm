attach "asm/std.asm"

entry:
  sdl_begin

  mov r5 5
  mov r6 5
  mov r7 0xFFF
  mov r8 message
  call sdl_render_str

  sdl_end
  jmp entry

message: defb "Hello, world!\0"

