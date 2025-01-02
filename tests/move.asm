
.loop
  mov *0xFFFA 1 // Clear screen

  mov r5 *.px
  mov r6 *.py
  mov r7 0
  call word .point

  call word .move

  mov *0xFFFB 1 // Display screen
  jmp word .loop

  halt

.move
  pusha

  mov r1 *.px
  mov r2 *.py

  mov r5 26 // W
  call word .is_down

  cmp ac 0
  jeq word ._move_skip_u
  sub r2 r2 1

._move_skip_u
  mov r5 22 // S
  call word .is_down

  cmp ac 0
  jeq word ._move_skip_d
  add r2 r2 1

._move_skip_d
  mov r5 4 // A
  call word .is_down

  cmp ac 0
  jeq word ._move_skip_l
  sub r1 r1 1

._move_skip_l
  mov r5 7 // D
  call word .is_down

  cmp ac 0
  jeq word ._move_skip_r
  add r1 r1 1

._move_skip_r
  mov *.px r1
  mov *.py r2

  popa
  ret

.is_down
  pusha
  
  add word r1 r5 0x7000
  mov ac *r1

  popa
  ret

.c2i
  pusha
  mov ac 0

  mul ac r6 128
  add ac ac r5
  add word ac ac 0x3000

  popa
  ret

.point
  pusha

  call word .c2i
  mov *ac r7

  popa
  ret

.px
  define 10

.py
  define 10

