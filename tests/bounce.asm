
.loop
  mov byte *0xFFFA 1 // Clear screen

  mov byte r5 *.px
  mov byte r6 *.py
  mov byte r7 *.color
  call .point

  call .contain
  call .update

  mov byte *0xFFFB 1 // Display screen
  jmp .loop

  halt

.c2i
  pusha
  mov ac 0

  mul ac r6 128
  add ac ac r5
  add ac ac 0x3000

  popa
  ret

.point
  pusha

  call .c2i
  mov byte *ac r7

  popa
  ret

.contain
  pusha

  mov byte r1 *.px
  mov byte r2 *.dx
  mov byte r3 *.py
  mov byte r4 *.dy

  cmp r1 126 // 128 - velocity
  jlt ._contain_skipl

  mov r2 1
  call .cycle_color

._contain_skipl 
  cmp r1 0
  jgt ._contain_skipr

  mov r2 0
  call .cycle_color

._contain_skipr
  cmp r3 127 // 128 - velocity
  jlt ._contain_skipd

  mov r4 1
  call .cycle_color

._contain_skipd
  cmp r3 0
  jgt ._contain_skipu

  mov r4 0
  call .cycle_color

._contain_skipu
  mov byte *.px r1
  mov byte *.dx r2
  mov byte *.py r3
  mov byte *.dy r4

  popa
  ret

.update
  pusha

  mov byte r1 *.px
  mov byte r2 *.vx
  mov byte r3 *.dx

  cmp r3 0
  jne ._update_subx

  add r1 r1 r2
  jmp ._update_conx

._update_subx
  sub r1 r1 r2

._update_conx
  mov byte r4 *.py
  mov byte r5 *.vy
  mov byte r6 *.dy

  cmp r6 0
  jne ._update_suby

  add r4 r4 r5
  jmp ._update_cony

._update_suby
  sub r4 r4 r5

._update_cony
  mov byte *.px r1
  mov byte *.vx r2
  mov byte *.dx r3

  mov byte *.py r4
  mov byte *.vy r5
  mov byte *.dy r6

  popa
  ret

.cycle_color
  pusha
  
  mov r1 *.color
  add byte r1 r1 1
  div byte r8 r1 115
  mov *.color ac

  popa
  ret

// Position
.px
  define byte 10

.py
  define byte 10

// Velocity
.vx
  define byte 2

.vy
  define byte 1

// Direction, since we don't have signed values!
.dx
  define byte 1

.dy
  define byte 0

.color
  define byte 0

