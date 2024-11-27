
mov r1 .value   ; Move address of ".value" into r1
mov @r1 16#abcd ; Modify memory at address held in r1
mov r2 @r1      ; Move value at address held in r1 into r2

halt

.value
  dw 0

