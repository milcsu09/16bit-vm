
push 16#ABCD
push 16#1234
call .function

halt

.function
  push bp
  mov bp sp

  mov ac bp
  add ac ac 6
  
  mov r1 @ac
  add ac ac 2

  mov r2 @ac

  mov sp bp
  pop bp
  ret

