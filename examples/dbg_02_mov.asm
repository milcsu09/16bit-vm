; If ran with vm-dbg, advised to execute the below commands.

; 'mv 0xDEAD 10 10 0
; 'mv 0xBEEF 10 10 0

; The command 'mv is read as "[m]emory [v]iew <address> <below> <above> <decode-mode>"
; where <address> is the address to view
;       <below> is the amount of bytes to view below <address>
;       <above> is the amount of bytes to view above <address>
;       <decode-mode> is the decoding mode
;       where 0 is "decode as nothing"
;             1 is "decode as instruction"
;             2 is "decode as ASCII character"

entry:
  ; *(0xBEEF + 0) = 0xDEAD & 0xFF
  ; *(0xBEEF + 0) = 0xAD
  ;
  ; *(0xBEEF + 1) = 0xDEAD >> 8
  ; *(0xBEEF + 1) = 0xDE
  mov [0xBEEF] 0xDEAD

  ; r1 = *(0xBEEF + 0) | (*(0xBEEF + 1) << 8)
  ; r1 = 0xAD          | (0xDE          << 8)
  ; r1 = 0xAD          | (0xDE00)
  ; r1 = 0xDEAD
  mov r1 [0xBEEF]

  ; *(r1     + 0) = *(0xBEEF + 0)
  ; *(0xDEAD + 0) = *(0xBEEF + 0)
  ; *(0xDEAD + 0) = 0xAD
  ;
  ; *(r1     + 1) = *(0xBEEF + 1)
  ; *(0xDEAD + 1) = *(0xBEEF + 1)
  ; *(0xDEAD + 1) = 0xDE
  mov [r1] [0xBEEF]

  halt

