# CALL CONVENTION : (r5, r6, r7, r8) -> ac

## STANDARD ##
std_strcpy: # (r5 src, r6 dst)
  pusha

std_strcpy_loop:
  mov r1 (r5)
  mov (r6) r1

  cmp r1 0
  jeq std_strcpy_end

  add r5 r5 1
  add r6 r6 1
  jmp std_strcpy_loop

std_strcpy_end:
  popa
  ret

## TTY ##
TTY_WRITER_ADDRESS = 0x3000

tty_write: # (r5 src)
  pusha

tty_write_loop:
  mov r1 (r5)

  cmp r1 0
  jeq tty_write_end

  mov (TTY_WRITER_ADDRESS) r1

  add r5 r5 1
  jmp tty_write_loop

tty_write_end:
  popa
  ret

