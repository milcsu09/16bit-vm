
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

.printm // (s) -> void
  pusha

._printm_loop
  mov byte r1 *r5
  cmp byte r1 0
  jeq ._printm_end

  mov r2 *._printm_pointer
  mov byte *r2 r1

  add byte r2 r2 1
  add byte r5 r5 1

  mov *._printm_pointer r2

  jmp ._printm_loop

._printm_end
  popa
  ret

// Points to the next free place to printm
._printm_pointer
  define 0x7000


