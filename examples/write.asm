
WRITE_BUFFER_ADDRESS = 0x3000

mov w r5 message
call write

halt

message: def "Hello, world!\n\0"

write:
  pusha

  mov w r1 WRITE_BUFFER_ADDRESS

write_loop:
  mov r2 (r5)

  cmp r2 '\0
  jeq write_end

  mov (r1) r2

  add r1 r1 1
  add r5 r5 1
  jmp write_loop

write_end:
  popa
  ret

