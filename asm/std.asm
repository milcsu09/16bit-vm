# STANDARD LIBRARY
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


std_strtoi: # (r5 src)
  pusha

  mov ac 0

std_strtoi_loop:
  mov r1 (r5)

  cmp r1 '\0
  jeq std_strtoi_end

  cmp r1 '0
  jlt std_strtoi_end

  cmp r1 '9
  jgt std_strtoi_end

  sub r1 r1 '0

  mul ac ac 10
  add ac ac r1

  add r5 r5 1
  jmp std_strtoi_loop

std_strtoi_end:
  popa
  ret


## TTY ##
TTY_WRITER_ADDRESS = 0x3000
TTY_READER_ADDRESS = 0x3100

tty_write = value
{
  mov (TTY_WRITER_ADDRESS) value
}

tty_writes: # (r5 src)
  pusha

tty_writes_loop:
  mov r1 (r5)

  cmp r1 0
  jeq tty_writes_end

  tty_write r1

  add r5 r5 1
  jmp tty_writes_loop

tty_writes_end:
  popa
  ret


tty_read =
{
  mov ac (TTY_READER_ADDRESS)
}

tty_reads: # (r5 dst, r6 size)
  pusha

  mov r1 0
  sub r6 r6 1

tty_reads_loop:
  tty_read

  cmp ac 0xFF
  jeq tty_reads_end

  cmp ac '\n
  jeq tty_reads_end

  cmp r1 r6
  jge tty_reads_loop

  mov (r5) ac

  add r1 r1 1
  add r5 r5 1
  jmp tty_reads_loop

tty_reads_end:
  mov (r5) '\0

  popa
  ret


