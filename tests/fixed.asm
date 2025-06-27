# Fixed-point library.

FP_BITS = 8
FP_ONE = [1 << FP_BITS]

itof = dest value
{
  shl dest value FP_BITS
}

ftoi = dest value
{
  add value value 0.5
  shr dest value FP_BITS
}


# TODO: Better multiplication
fmul = dest src1 src2
{
  push r1
  push r2

  shr w r1 src1 4
  shr w r2 src2 4

  mul w dest r1 r2

  pop r2
  pop r1
}


fdiv:
  pusha

  # result
  mov w r1 0

  # bit
  mov w r2 0x8000

fdiv_loop:
  # bit != 0
  cmp w r2 0
  jle fdiv_end

  # high << 1
  mov w r3 (fixed_h)
  shl r3 r3 1

  # low >> 15
  mov w r4 (fixed_l)
  shr r4 r4 15

  # (high << 1) | (low >> 15)
  or r3 r3 r4

  # high = (high << 1) | (low >> 15)
  mov w (fixed_h) r3

  # low = low << 1
  mov w r3 (fixed_l)
  shl r3 r3 1
  mov w (fixed_l) r3

  mov w r3 (fixed_h)

  cmp w r3 r5
  jlt fdiv_skip_subtract

  sub r3 r3 r5
  mov w (fixed_h) r3

  or r1 r1 r2

fdiv_skip_subtract:
  # bit >>= 1
  shr r2 r2 1
  jmp fdiv_loop

fdiv_end:
  mov w ac r1

  popa
  ret


fset:
  push r1

  mov r1 r5
  and r1 r1 0xFF
  shl r1 r1 8
  mov w (fixed_l) r1

  mov r1 r5
  shr r1 r1 8
  mov w (fixed_h) r1

  pop r1

  ret


fixed_l: res w 1
fixed_h: res w 1


