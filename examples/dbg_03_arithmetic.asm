entry:
  mov r1 34

  ; r2 = r1 + 35
  ; r2 = 69
  add r2 r1 35

  ; r3 = r1 + r2
  ; r3 = 34 + 69
  ; r3 = 103
  add r3 r1 r2

  ; r4 = r3  - 33
  ; r4 = 103 - 33
  ; r4 = 70
  sub r4 r3 33

  ; r5 = r4 * 6
  ; r5 = 70 * 6
  ; r5 = 420
  mul r5 r4 6

  ; r6 = r5  / r2
  ; r6 = 420 / 69
  ; r6 = 6
  ; ac = r5  % r2
  ; ac = 420 % 69
  ; ac = 6
  div r6 r5 r2

  ; r7 = r6 * ac
  ; r7 = 6  * 6
  ; r7 = 36
  mul r7 r6 ac

  halt

