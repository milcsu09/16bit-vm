
nop

mov r1 1024
mov r2 r1
mov r3 @16#3000
mov r4 @r3

mov @16#3000 1024
mov @16#3000 r1
mov @16#3000 @16#4000
mov @16#3000 @r3

mov @r4 1024
mov @r4 r1
mov @r4 @16#4000
mov @r4 @r3

push 512
push r4

pop r2
pop r2

add r1 r1 512
add r1 r1 r2

sub r1 r1 512
sub r1 r1 r2

mul r1 r1 512
mul r1 r1 r2

div r1 r1 512
div r1 r1 r2

mov r2 @.function

.function

halt

