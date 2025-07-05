
attach "asm/std.asm"

entry:
loop:
  sdl_begin

  mov r5 10
  mov r6 10
  mov r7 0xFAF
  mov r8 message
  call sdl_render_str

  sdl_end
  jmp loop

halt

message: defb "123 456 789" 0

