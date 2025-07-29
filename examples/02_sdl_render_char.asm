
attach "asm/std.asm"

entry:
loop:
  sdl_begin

  mov r5 10
  mov r6 10
  mov r7 0x77F
  mov r8 s1
  call sdl_render_str

  mov r5 10
  mov r6 18
  mov r7 0xF77
  mov r8 s2
  call sdl_render_str

  mov r5 10
  mov r6 27
  mov r7 0x7F7
  mov r8 s3
  call sdl_render_str

  sdl_end
  jmp loop

halt

s1: defb "Hello, world!" 0
s2: defb "^~~~~  ^~~~~" 0

s3: defb "This is a really really really long and pointless paragraph about nothing." 0

