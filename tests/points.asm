
attach "tests/sdl.asm"

N = 1024

# NOTE: 'at' mustn't be r7 or r8
subscript = bits from at to
{
  pusha

  # 8 (bits=0) or 16 (bits=1) bit mode.
  # r7 = bits + 1
  mov r7 bits
  add r7 r7 1

  # Compute offset amount.
  # r8 = at * r7
  mov w r8 at
  mul w r8 r8 r7

  # Compute static address.
  # ac = from + r8
  mov w ac from
  add w ac ac r8

  popa

  mov w to ac
}

hash = x to
{
  mul w to x 0xA297

  mov w r7 to
  shr w r7 r7 7
  xor w to to r7

  mul w to to 0x79B1
}

__main_init:
  mov w r1 0

test_loop:
  cmp w r1 N
  jeq test_loop_end

  ###############

  subscript 0 pxs r1 r5
  subscript 0 pys r1 r6

  hash r1 r2
  mov w r3 r2
  and w r3 r3 127

  mov (r5) r3

  hash r2 r2
  mov w r3 r2
  and w r3 r3 127

  mov (r6) r3

  ###############

  subscript 0 vxs r1 r5
  subscript 0 vys r1 r6

  hash r1 r2
  mov w r3 r2
  and w r3 r3 1

  mov (r5) r3

  hash r2 r2
  mov w r3 r2
  and w r3 r3 1

  mov (r6) r3

  ###############

  subscript 1 clr r1 r5

  hash r1 r2
  and w r2 r2 0xFFF

  mov w (r5) r2

  add r1 r1 1
  jmp test_loop

test_loop_end:


main:
  mov (FLAG_ADDRESS_EVENT) 1
  mov (FLAG_ADDRESS_CLEAR) 1

  mov w r1 0

main_draw_loop:
  cmp w r1 N
  jge main_draw_loop_end

  subscript 0 pxs r1 r5
  subscript 0 pys r1 r6

  subscript 0 vxs r1 r7
  subscript 0 vys r1 r8

  push w r7
  push w r8

  # r5, r6 already setup
  mov r7 (r7)
  mov r8 (r8)
  call move

  pop w r8
  pop w r7

  # r7, r8 already setup
  mov r5 (r5)
  mov r6 (r6)
  call contain

  # r5, r6 already setup
  subscript 1 clr r1 r7
  mov w r7 (r7)
  # mov w r7 0xfff
  call renderer_draw_point

  add r1 r1 1
  jmp main_draw_loop

main_draw_loop_end:
  mov (FLAG_ADDRESS_DISPLAY) 1
  jmp main

  halt

move: # *x *y vx vy
  pusha

  mov r1 (r5)
  mov r2 (r6)

  cmp r7 0
  jne move_left

  add r1 r1 1
  jmp move_left_skip

move_left:
  sub r1 r1 1

move_left_skip:
  cmp r8 0
  jne move_up

  add r2 r2 1
  jmp move_up_skip

move_up:
  sub r2 r2 1

move_up_skip:
  mov (r5) r1
  mov (r6) r2

  popa
  ret

contain: # x, y, *vx, *vy
  pusha

  mov r1 (r7)
  mov r2 (r8)

  cmp r5 127
  jne contain_left

  mov r1 1
  jmp contain_left_skip

contain_left:
  cmp r5 0
  jne contain_left_skip

  mov r1 0

contain_left_skip:

  cmp r6 127
  jne contain_right

  mov r2 1
  jmp contain_right_skip

contain_right:
  cmp r6 0
  jne contain_right_skip

  mov r2 0

contain_right_skip:

  mov (r7) r1
  mov (r8) r2

  popa
  ret

pxs: res N
pys: res N

vxs: res N
vys: res N

clr: res w N

