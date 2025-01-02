
  call word .init_points


.loop
  mov *0xFFFA 1

  call word .move_points
  call word .bounce_points
  call word .draw_points

  mov *0xFFFB 1

  jmp word .loop
  halt


.init_points
  pusha

  mov r1 0

.init_points_loop
  cmp r1 64
  jge word .init_points_end

  mov word r5 .xs
  add r5 r5 r1

  mov word r6 .ys
  add r6 r6 r1

  mov word r7 .dxs
  add r7 r7 r1

  mov word r8 .dys
  add r8 r8 r1

  mul r2 r1 2
  mov *r6 r2

  mov r3 127
  mul r2 r1 2
  sub r2 r3 r2
  mov *r5 r2

  div r2 r1 2
  mov *r7 ac
  mov *r8 ac

  add r1 r1 1
  jmp word .init_points_loop

.init_points_end
  popa
  ret


.move_points
  pusha

  mov r1 0

.move_points_loop
  cmp r1 64
  jge word .move_points_end

  mov word r5 .xs
  add r5 r5 r1

  mov word r6 .dxs
  add r6 r6 r1

  mov r2 *r5
  mov r3 *r6

  cmp r3 0
  jne word .move_points_sub_x

  add r2 r2 1
  jmp word .move_points_add_x

.move_points_sub_x
  sub r2 r2 1

.move_points_add_x
  mov *r5 r2

  mov word r5 .ys
  add r5 r5 r1

  mov word r6 .dys
  add r6 r6 r1

  mov r2 *r5
  mov r3 *r6

  cmp r3 0
  jne word .move_points_sub_y

  add r2 r2 1
  jmp word .move_points_add_y

.move_points_sub_y
  sub r2 r2 1

.move_points_add_y
  mov *r5 r2

  add r1 r1 1
  jmp word .move_points_loop

.move_points_end
  popa
  ret


.bounce_points
  pusha

  mov r1 0

.bounce_points_loop
  cmp r1 64
  jge word .bounce_points_end

  mov word r5 .xs
  add r5 r5 r1

  mov word r6 .ys
  add r6 r6 r1

  mov word r7 .dxs
  add r7 r7 r1

  mov word r8 .dys
  add r8 r8 r1

  mov r5 *r5
  mov r6 *r6

  cmp r5 0 // LEFT
  jgt word .bounce_points_left

  mov *r7 0
  mov *r8 0

.bounce_points_left
  cmp r5 127 // RIGHT
  jlt word .bounce_points_right

  mov *r7 1
  mov *r8 1

.bounce_points_right
  cmp r6 0 // UP
  jgt word .bounce_points_up

  mov *r7 0
  mov *r8 0

.bounce_points_up
  cmp r6 127 // DOWN
  jlt word .bounce_points_down

  mov *r7 1
  mov *r8 1

.bounce_points_down
  add r1 r1 1
  jmp word .bounce_points_loop

.bounce_points_end
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
  mov *ac r7

  popa
  ret


.draw_points
  pusha

  mov r1 0

.draw_points_loop
  cmp r1 64
  jge word .draw_points_end

  mov word r5 .xs
  add r5 r5 r1

  mov word r6 .ys
  add r6 r6 r1

  mov r5 *r5
  mov r6 *r6
  mov r7 r1
  call word .draw_point

  add r1 r1 1

  jmp word .draw_points_loop

.draw_points_end
  popa
  ret


.xs
  reserve 64

.ys
  reserve 64

.dxs
  reserve 64

.dys
  reserve 64

