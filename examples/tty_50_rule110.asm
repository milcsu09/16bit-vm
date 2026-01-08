attach "asm/std.asm"

N = 80

entry:
  mov r1 cells_curr
  add r1 r1 (N - 1)
  movb [r1] 1

  mov r1 0

loop_step:
  cmp r1 N
  jge end_step

  call print_cells

  mov r2 0

loop_update:
  cmp r2 N
  jge end_update

  mov r6 cells_curr
  add r6 r6 r2
  movb r6 [r6]

  cmp r2 0
  jeq loop_update_left

  mov r5 cells_curr
  sub r3 r2 1
  add r5 r5 r3
  movb r5 [r5]

  jmp loop_update_not_left

loop_update_left:
  mov r5 0

loop_update_not_left:
  cmp r2 (N - 1)
  jeq loop_update_right

  mov r7 cells_curr
  add r3 r2 1
  add r7 r7 r3
  movb r7 [r7]

  jmp loop_update_not_right

loop_update_right:
  mov r7 0

loop_update_not_right:
  call rule110

  mov r3 cells_next
  add r3 r3 r2
  movb [r3] ac

  add r2 r2 1
  jmp loop_update

end_update:
  mov r2 0

loop_copy:
  cmp r2 N
  jge end_copy

  mov r3 cells_curr
  mov r4 cells_next

  add r3 r3 r2
  add r4 r4 r2

  movb [r3] [r4]

  add r2 r2 1
  jmp loop_copy

end_copy:

  add r1 r1 1
  jmp loop_step

end_step:
  halt


rule110: ; (l: r5, m: r6, r: r7) -> ac
  pusha

  shl r2 r5 2
  shl r3 r6 1

  or r1 r2 r3
  or r1 r1 r7

  mov ac rule110_table
  add ac ac r1
  movb ac [ac]

rule110_end:
  popa
  ret


rule110_table:
  defb 0 1 1 1 0 1 1 0


print_cells:
  pusha
  mov r1 0

print_cells_loop:
  cmp r1 N
  jge print_cells_end

  mov r2 cells_curr
  add r2 r2 r1
  movb r2 [r2]

  cmp r2 1
  jne print_cells_space

  tty_write '.

  jmp print_cells_dot

print_cells_space:
  tty_write ' 

print_cells_dot:
  add r1 r1 1
  jmp print_cells_loop

print_cells_end:
  tty_write 10

  popa
  ret


cells_curr: resb N
cells_next: resb N

