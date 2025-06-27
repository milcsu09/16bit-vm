# EXTENDED STANDARD LIBRARY
# CALL CONVENTION : (r5, r6, r7, r8) -> ac

STDX_FBITS = 8
STDX_FONE = [1 << STDX_FBITS]

stdx_itof = dst src
{
  shl w dst src STDX_FBITS
}

stdx_ftoi = dst src
{
  shr w dst src STDX_FBITS
}


