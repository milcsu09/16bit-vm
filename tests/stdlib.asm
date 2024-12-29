
// Standard library of the virtual machine's assembly language.
// Here you'll find some quality of life utility functions.
// The call convention is (r5, r6, r7, r8, stack, ...) -> ac

.mset // (dest, value, n) -> void
  pusha

._mset_loop
  cmp byte r7 0
  jeq ._mset_end

  mov byte *r5 r6
  add byte r5 r5 1
  sub byte r7 r7 1

  jmp ._mset_loop

._mset_end
  popa
  ret

.mcpy // (dest, src, n) -> void
  pusha

._mcpy_loop
  cmp byte r7 0
  jeq ._mcpy_end

  mov byte *r5 *r6
  add byte r5 r5 1
  add byte r6 r6 1
  sub byte r7 r7 1

  jmp ._mcpy_loop

._mcpy_end
  popa
  ret

.mlen // (s) -> len
  pusha
  mov byte ac 0

._mlen_loop
  mov byte r1 *r5
  cmp byte r1 0
  jeq ._mlen_end

  add byte r5 r5 1
  add byte ac ac 1

  jmp ._mlen_loop

._mlen_end
  popa
  ret


