
.loop
  mov byte *0xFFFA 1 // Clear screen

  mov byte r5 *.px
  mov byte r6 *.py
  mov r7 0
  call .point

  call .move

  mov byte *0xFFFB 1 // Display screen
  jmp .loop

  halt

.move
  pusha

  mov r1 *.px
  mov r2 *.py

  mov byte r5 26 // W
  call .is_down

  cmp ac 0
  jeq ._move_skip_u
  sub byte r2 r2 1

._move_skip_u
  mov byte r5 22 // S
  call .is_down

  cmp ac 0
  jeq ._move_skip_d
  add byte r2 r2 1

._move_skip_d
  mov byte r5 4 // A
  call .is_down

  cmp ac 0
  jeq ._move_skip_l
  sub byte r1 r1 1

._move_skip_l
  mov byte r5 7 // D
  call .is_down

  cmp ac 0
  jeq ._move_skip_r
  add byte r1 r1 1

._move_skip_r
  mov *.px r1
  mov *.py r2

  popa
  ret

.is_down
  pusha
  
  add r1 r5 0x7000
  mov byte ac *r1

  popa
  ret

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

.px
  define byte 10

.py
  define byte 10

