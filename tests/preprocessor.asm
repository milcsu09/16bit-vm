
%define stack-frame-create
  push bp
  mov bp sp
%end

%define stack-frame-destroy
  mov sp bp
  pop bp
%end

.function
  %stack-frame-create

  %stack-frame-destroy
  ret

