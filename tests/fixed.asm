# Fixed-point library.

FP_BITS = 8
FP_ONE = [1 << FP_BITS]

itof = dest value
{
  shl dest value FP_BITS
}

ftoi = dest value
{
  shr dest value FP_BITS
}

fmul = dest src1 src2
{
  mul dest src1 src2
}

fdiv = dest src1 src2
{
  div dest dest src2
}

