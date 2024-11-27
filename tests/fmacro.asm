
%define ret
  pop ip
%end

%define stack-frame-create
  push bp
  mov bp sp
%end

%define stack-frame-destroy
  mov sp bp
  pop bp
%end

halt

.function
  %stack-frame-create
  %stack-frame-destroy
  %ret

