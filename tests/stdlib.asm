
// Standard library of the virtual machine's assembly language.
// Here you'll find some quality of life utility functions.
// The call convention is (r5, r6, r7, r8, stack, ...) -> ac

.mset // (address, value, n) -> void
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

