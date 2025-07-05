
attach "asm/std.asm"

entry:
loop:
  sdl_begin

  mov r5 10
  mov r6 10
  mov r7 0xF77
  mov r8 s1
  call sdl_render_str

  mov r5 10
  mov r6 40
  mov r7 0x7F7
  mov r8 s2
  call sdl_render_str

  sdl_end
  jmp loop

halt

s1: defb "Hello, world!" 0
s2: defb "2 + 2 = 2 ^ 2" 0

