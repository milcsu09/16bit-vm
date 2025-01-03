
.loop
  mov *0xFFFA 1

  mov r5 10
  mov r6 10
  mov r7 30
  mov r8 30
  push word 0xFAF
  call word .rect

  mov r5 30
  mov r6 30
  mov r7 45
  mov r8 32
  push word 0xAFA
  call word .rect

  add sp sp 2

  mov *0xFFFB 1
  jmp word .loop

  halt

.rect
  pusha

  mov r3 sp
  add r3 r3 20

  sub r5 r5 1
  sub r6 r6 1

  add r7 r7 1
  add r8 r8 1

.rect_outer
  cmp r7 0
  jeq word .rect_outer_end

  push r8

.rect_inner
  cmp r8 0
  jeq word .rect_inner_end

  push r5
  push r6
  push r7

  add r5 r5 r7
  add r6 r6 r8

  mov word r7 *r3
  call word .draw_point

  pop r7
  pop r6
  pop r5

  sub r8 r8 1
  jmp word .rect_inner

.rect_inner_end
  pop r8

  sub r7 r7 1
  jmp word .rect_outer

.rect_outer_end
  popa
  ret


.point_to_index
  pusha
  mov ac 0

  mul ac r6 128
  add ac ac r5
  add word ac ac 0x3000

  popa
  ret


.draw_point
  pusha

  call word .point_to_index
  mov word *ac r7

  popa
  ret

