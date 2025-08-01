__lea = d b o s
{
  ; Base + Offset * Size
  mul d o s
  add d d b
}

entry:
  ; Offset
  mov r1 3

  __lea r2 table r1 2

  mov r3 [r2]

  ; Should call to function4!
  call r3

  ; r8 should be 40!

  halt

function1:
  mov r8 10
  ret

function2:
  mov r8 20
  ret

function3:
  mov r8 30
  ret

function4:
  mov r8 40
  ret

function5:
  mov r8 50
  ret

table:
  def function1
  def function2
  def function3
  def function4
  def function5

