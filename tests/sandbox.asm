
%const sizeof(word) 2
%const IO 16#3000

mov ac %IO

mov r1 'H'
call .write-char
mov r1 'e'
call .write-char
mov r1 'l'
call .write-char
mov r1 'l'
call .write-char
mov r1 'o'
call .write-char
mov r1 '!'
call .write-char
mov r1 10
call .write-char

halt

.write-char ; (c: r1) -> void
  mov @ac r1
  add ac ac %sizeof(word)
  ret

