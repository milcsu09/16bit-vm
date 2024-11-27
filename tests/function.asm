
%define stack-frame-create
%end

%define stack-frame-destroy
  mov sp bp
  pop bp
%end

push 16#1234
push 16#ABCD
call .function

halt

.function
  push bp

  mov r1 bp
  sub r1 r1 1
  mov r1 @r1

  mov r1 bp
  sub r1 r1 3
  mov r1 @r1

  mov bp sp

  %stack-frame-destroy
  ret

