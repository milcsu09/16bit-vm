
  call .init_points


.loop
  mov byte *0xFFFA 1

  call .move_points
  call .bounce_points
  call .draw_points

  mov byte *0xFFFB 1

  jmp .loop
  halt


.init_points
  pusha

  mov byte r1 0

.init_points_loop
  cmp byte r1 64
  jge .init_points_end

  mov r5 .xs
  add r5 r5 r1

  mov r6 .ys
  add r6 r6 r1

  mov r7 .dxs
  add r7 r7 r1

  mov r8 .dys
  add r8 r8 r1

  mul byte r2 r1 2
  mov byte *r6 r2

  mov byte r3 127
  mul byte r2 r1 2
  sub byte r2 r3 r2
  mov byte *r5 r2

  div byte r2 r1 2
  mov byte *r7 ac
  mov byte *r8 ac

  add byte r1 r1 1
  jmp .init_points_loop

.init_points_end
  popa
  ret


.move_points
  pusha

  mov byte r1 0

.move_points_loop
  cmp byte r1 64
  jge .move_points_end

  mov r5 .xs
  add r5 r5 r1

  mov r6 .dxs
  add r6 r6 r1

  mov byte r2 *r5
  mov byte r3 *r6

  cmp byte r3 0
  jne .move_points_sub_x

  add byte r2 r2 1
  jmp .move_points_add_x

.move_points_sub_x
  sub byte r2 r2 1

.move_points_add_x
  mov byte *r5 r2

  mov r5 .ys
  add r5 r5 r1

  mov r6 .dys
  add r6 r6 r1

  mov byte r2 *r5
  mov byte r3 *r6

  cmp byte r3 0
  jne .move_points_sub_y

  add byte r2 r2 1
  jmp .move_points_add_y

.move_points_sub_y
  sub byte r2 r2 1

.move_points_add_y
  mov byte *r5 r2

  add byte r1 r1 1
  jmp .move_points_loop

.move_points_end
  popa
  ret


.bounce_points
  pusha

  mov byte r1 0

.bounce_points_loop
  cmp byte r1 64
  jge .bounce_points_end

  mov r5 .xs
  add r5 r5 r1

  mov r6 .ys
  add r6 r6 r1

  mov r7 .dxs
  add r7 r7 r1

  mov r8 .dys
  add r8 r8 r1

  mov byte r5 *r5
  mov byte r6 *r6

  cmp byte r5 0 // LEFT
  jgt .bounce_points_left

  mov byte *r7 0
  mov byte *r8 0

.bounce_points_left
  cmp byte r5 127 // RIGHT
  jlt .bounce_points_right

  mov byte *r7 1
  mov byte *r8 1

.bounce_points_right
  cmp byte r6 0 // UP
  jgt .bounce_points_up

  mov byte *r7 0
  mov byte *r8 0

.bounce_points_up
  cmp byte r6 127 // DOWN
  jlt .bounce_points_down

  mov byte *r7 1
  mov byte *r8 1

.bounce_points_down
  add byte r1 r1 1
  jmp .bounce_points_loop

.bounce_points_end
  popa
  ret


.point_to_index
  pusha
  mov byte ac 0

  mul byte ac r6 128
  add ac ac r5
  add ac ac 0x3000

  popa
  ret


.draw_point
  pusha

  call .point_to_index
  mov byte *ac r7

  popa
  ret


.draw_points
  pusha

  mov byte r1 0

.draw_points_loop
  cmp byte r1 64
  jge .draw_points_end

  mov r5 .xs
  add r5 r5 r1

  mov r6 .ys
  add r6 r6 r1

  mov byte r5 *r5
  mov byte r6 *r6
  mov byte r7 r1
  call .draw_point

  add byte r1 r1 1

  jmp .draw_points_loop

.draw_points_end
  popa
  ret


.xs
  reserve byte 64

.ys
  reserve byte 64

.dxs
  reserve byte 64

.dys
  reserve byte 64

