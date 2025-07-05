# STANDARD LIBRARY
# CALL CONVENTION : (r5, r6, r7, r8, .. stack) -> ac


## STANDARD ##
std_neg = value
{
  push ac
  mov ac 0
  sub value ac value
  pop ac
}

std_lnot = dst value
{
  push r1
  push r2

  not r1 value
  sub r2 value 1

  and r1 r1 r2
  shr r1 r1 15

  and dst r1 1

  pop r2
  pop r1
}

std_strcpy: # (r5 src, r6 dst)
  pusha

std_strcpy_loop:
  movb r1 (r5)
  movb (r6) r1

  cmp r1 0
  jeq std_strcpy_end

  add r5 r5 1
  add r6 r6 1
  jmp std_strcpy_loop

std_strcpy_end:
  popa
  ret


std_strtoi: # (r5 src)
  pusha

  mov ac 0

std_strtoi_loop:
  movb r1 (r5)

  cmp r1 '\0
  jeq std_strtoi_end

  cmp r1 '0
  jlt std_strtoi_end

  cmp r1 '9
  jgt std_strtoi_end

  sub r1 r1 '0

  mul ac ac 10
  add ac ac r1

  add r5 r5 1
  jmp std_strtoi_loop

std_strtoi_end:
  popa
  ret


std_itoa: # (r5 value, r6 buffer)
  pusha

  cmp r5 0
  jeq std_itoa_zero

  mov r1 0

std_itoa_loop:
  cmp r5 0
  jeq std_itoa_end

  add r2 r1 __std_itoa_buffer
  add r1 r1 1

  div r5 r5 10

  add ac ac '0
  movb (r2) ac

  jmp std_itoa_loop

std_itoa_end:
  mov r2 0

std_itoa_reverse_loop:
  cmp r1 0
  jeq std_itoa_reverse_end

  add r3 r2 r6
  add r2 r2 1

  sub r1 r1 1
  add r4 r1 __std_itoa_buffer

  movb (r3) (r4)

  jmp std_itoa_reverse_loop

std_itoa_reverse_end:
  add r3 r2 r6
  movb (r3) '\0

  popa
  ret

std_itoa_zero:
  mov (r6) '0

  add r6 r6 1
  mov (r6) '\0

  popa
  ret


# Utility buffer to hold the temporary digits produced by 'std_itoa'.
__std_itoa_buffer: resb 5


## TTY ##
TTY_WRITER_ADDRESS = 0x3000
TTY_READER_ADDRESS = 0x3100

tty_write = value
{
  movb (TTY_WRITER_ADDRESS) value
}

tty_writes: # (r5 src)
  pusha

tty_writes_loop:
  movb r1 (r5)

  cmp r1 0
  jeq tty_writes_end

  tty_write r1

  add r5 r5 1
  jmp tty_writes_loop

tty_writes_end:
  popa
  ret


tty_read =
{
  movb ac (TTY_READER_ADDRESS)
}

tty_reads: # (r5 dst, r6 size)
  pusha

  movb r1 0
  sub r6 r6 1

tty_reads_loop:
  tty_read

  cmp ac 0xFF
  jeq tty_reads_end

  cmp ac '\n
  jeq tty_reads_end

  cmp r1 r6
  jge tty_reads_loop

  movb (r5) ac

  add r1 r1 1
  add r5 r5 1
  jmp tty_reads_loop

tty_reads_end:
  movb (r5) '\0

  popa
  ret


## SDL ##
SDL_SCREEN_SIZE = 128

SDL_RENDERER_ADDRESS = 0x3000
SDL_KEYBOARD_ADDRESS = 0x7000

SDL_FLAG_BEGIN = 0x9000
SDL_FLAG_END   = 0x9001

sdl_begin =
{
  movb (SDL_FLAG_BEGIN) 1
}

sdl_end =
{
  movb (SDL_FLAG_END) 1
}

sdl_xy_to_address = x y
{
  mov ac 0
  mul ac y  SDL_SCREEN_SIZE
  add ac ac x
  add ac ac SDL_RENDERER_ADDRESS
}

sdl_render_point: # (r5 x, r6 y, r7 color)
  cmp r5 SDL_SCREEN_SIZE
  jge sdl_render_point_end

  cmp r6 SDL_SCREEN_SIZE
  jge sdl_render_point_end

  push ac

  sdl_xy_to_address r5 r6
  mov (ac) r7

  pop ac

sdl_render_point_end:
  ret


sdl_render_vline: # (r5 x, r6 y1, r7 y2, r8 color)
sdl_render_vline_loop:
  cmp r6 r7
  jgt sdl_render_vline_end

  push r7

  mov r7 r8
  call sdl_render_point

  pop r7

  add r6 r6 1
  jmp sdl_render_vline_loop

sdl_render_vline_end:
  ret


# Values from `SDL2/SDL_scancode.h`.
SDL_SCANCODE_UNKNOWN = 0
SDL_SCANCODE_A = 4
SDL_SCANCODE_B = 5
SDL_SCANCODE_C = 6
SDL_SCANCODE_D = 7
SDL_SCANCODE_E = 8
SDL_SCANCODE_F = 9
SDL_SCANCODE_G = 10
SDL_SCANCODE_H = 11
SDL_SCANCODE_I = 12
SDL_SCANCODE_J = 13
SDL_SCANCODE_K = 14
SDL_SCANCODE_L = 15
SDL_SCANCODE_M = 16
SDL_SCANCODE_N = 17
SDL_SCANCODE_O = 18
SDL_SCANCODE_P = 19
SDL_SCANCODE_Q = 20
SDL_SCANCODE_R = 21
SDL_SCANCODE_S = 22
SDL_SCANCODE_T = 23
SDL_SCANCODE_U = 24
SDL_SCANCODE_V = 25
SDL_SCANCODE_W = 26
SDL_SCANCODE_X = 27
SDL_SCANCODE_Y = 28
SDL_SCANCODE_Z = 29
SDL_SCANCODE_1 = 30
SDL_SCANCODE_2 = 31
SDL_SCANCODE_3 = 32
SDL_SCANCODE_4 = 33
SDL_SCANCODE_5 = 34
SDL_SCANCODE_6 = 35
SDL_SCANCODE_7 = 36
SDL_SCANCODE_8 = 37
SDL_SCANCODE_9 = 38
SDL_SCANCODE_0 = 39
SDL_SCANCODE_RETURN = 40
SDL_SCANCODE_ESCAPE = 41
SDL_SCANCODE_BACKSPACE = 42
SDL_SCANCODE_TAB = 43
SDL_SCANCODE_SPACE = 44
SDL_SCANCODE_MINUS = 45
SDL_SCANCODE_EQUALS = 46
SDL_SCANCODE_LEFTBRACKET = 47
SDL_SCANCODE_RIGHTBRACKET = 48
SDL_SCANCODE_BACKSLASH = 49
SDL_SCANCODE_NONUSHASH = 50
SDL_SCANCODE_SEMICOLON = 51
SDL_SCANCODE_APOSTROPHE = 52
SDL_SCANCODE_GRAVE = 53
SDL_SCANCODE_COMMA = 54
SDL_SCANCODE_PERIOD = 55
SDL_SCANCODE_SLASH = 56
SDL_SCANCODE_CAPSLOCK = 57
SDL_SCANCODE_F1 = 58
SDL_SCANCODE_F2 = 59
SDL_SCANCODE_F3 = 60
SDL_SCANCODE_F4 = 61
SDL_SCANCODE_F5 = 62
SDL_SCANCODE_F6 = 63
SDL_SCANCODE_F7 = 64
SDL_SCANCODE_F8 = 65
SDL_SCANCODE_F9 = 66
SDL_SCANCODE_F10 = 67
SDL_SCANCODE_F11 = 68
SDL_SCANCODE_F12 = 69
SDL_SCANCODE_PRINTSCREEN = 70
SDL_SCANCODE_SCROLLLOCK = 71
SDL_SCANCODE_PAUSE = 72
SDL_SCANCODE_INSERT = 73
SDL_SCANCODE_HOME = 74
SDL_SCANCODE_PAGEUP = 75
SDL_SCANCODE_DELETE = 76
SDL_SCANCODE_END = 77
SDL_SCANCODE_PAGEDOWN = 78
SDL_SCANCODE_RIGHT = 79
SDL_SCANCODE_LEFT = 80
SDL_SCANCODE_DOWN = 81
SDL_SCANCODE_UP = 82
SDL_SCANCODE_NUMLOCKCLEAR = 83
SDL_SCANCODE_KP_DIVIDE = 84
SDL_SCANCODE_KP_MULTIPLY = 85
SDL_SCANCODE_KP_MINUS = 86
SDL_SCANCODE_KP_PLUS = 87
SDL_SCANCODE_KP_ENTER = 88
SDL_SCANCODE_KP_1 = 89
SDL_SCANCODE_KP_2 = 90
SDL_SCANCODE_KP_3 = 91
SDL_SCANCODE_KP_4 = 92
SDL_SCANCODE_KP_5 = 93
SDL_SCANCODE_KP_6 = 94
SDL_SCANCODE_KP_7 = 95
SDL_SCANCODE_KP_8 = 96
SDL_SCANCODE_KP_9 = 97
SDL_SCANCODE_KP_0 = 98
SDL_SCANCODE_KP_PERIOD = 99
SDL_SCANCODE_NONUSBACKSLASH = 100
SDL_SCANCODE_APPLICATION = 101
SDL_SCANCODE_POWER = 102
SDL_SCANCODE_KP_EQUALS = 103
SDL_SCANCODE_F13 = 104
SDL_SCANCODE_F14 = 105
SDL_SCANCODE_F15 = 106
SDL_SCANCODE_F16 = 107
SDL_SCANCODE_F17 = 108
SDL_SCANCODE_F18 = 109
SDL_SCANCODE_F19 = 110
SDL_SCANCODE_F20 = 111
SDL_SCANCODE_F21 = 112
SDL_SCANCODE_F22 = 113
SDL_SCANCODE_F23 = 114
SDL_SCANCODE_F24 = 115
SDL_SCANCODE_EXECUTE = 116
SDL_SCANCODE_HELP = 117
SDL_SCANCODE_MENU = 118
SDL_SCANCODE_SELECT = 119
SDL_SCANCODE_STOP = 120
SDL_SCANCODE_AGAIN = 121
SDL_SCANCODE_UNDO = 122
SDL_SCANCODE_CUT = 123
SDL_SCANCODE_COPY = 124
SDL_SCANCODE_PASTE = 125
SDL_SCANCODE_FIND = 126
SDL_SCANCODE_MUTE = 127
SDL_SCANCODE_VOLUMEUP = 128
SDL_SCANCODE_VOLUMEDOWN = 129
SDL_SCANCODE_KP_COMMA = 133
SDL_SCANCODE_KP_EQUALSAS400 = 134
SDL_SCANCODE_INTERNATIONAL1 = 135
SDL_SCANCODE_INTERNATIONAL2 = 136
SDL_SCANCODE_INTERNATIONAL3 = 137
SDL_SCANCODE_INTERNATIONAL4 = 138
SDL_SCANCODE_INTERNATIONAL5 = 139
SDL_SCANCODE_INTERNATIONAL6 = 140
SDL_SCANCODE_INTERNATIONAL7 = 141
SDL_SCANCODE_INTERNATIONAL8 = 142
SDL_SCANCODE_INTERNATIONAL9 = 143
SDL_SCANCODE_LANG1 = 144
SDL_SCANCODE_LANG2 = 145
SDL_SCANCODE_LANG3 = 146
SDL_SCANCODE_LANG4 = 147
SDL_SCANCODE_LANG5 = 148
SDL_SCANCODE_LANG6 = 149
SDL_SCANCODE_LANG7 = 150
SDL_SCANCODE_LANG8 = 151
SDL_SCANCODE_LANG9 = 152
SDL_SCANCODE_ALTERASE = 153
SDL_SCANCODE_SYSREQ = 154
SDL_SCANCODE_CANCEL = 155
SDL_SCANCODE_CLEAR = 156
SDL_SCANCODE_PRIOR = 157
SDL_SCANCODE_RETURN2 = 158
SDL_SCANCODE_SEPARATOR = 159
SDL_SCANCODE_OUT = 160
SDL_SCANCODE_OPER = 161
SDL_SCANCODE_CLEARAGAIN = 162
SDL_SCANCODE_CRSEL = 163
SDL_SCANCODE_EXSEL = 164
SDL_SCANCODE_KP_00 = 176
SDL_SCANCODE_KP_000 = 177
SDL_SCANCODE_THOUSANDSSEPARATOR = 178
SDL_SCANCODE_DECIMALSEPARATOR = 179
SDL_SCANCODE_CURRENCYUNIT = 180
SDL_SCANCODE_CURRENCYSUBUNIT = 181
SDL_SCANCODE_KP_LEFTPAREN = 182
SDL_SCANCODE_KP_RIGHTPAREN = 183
SDL_SCANCODE_KP_LEFTBRACE = 184
SDL_SCANCODE_KP_RIGHTBRACE = 185
SDL_SCANCODE_KP_TAB = 186
SDL_SCANCODE_KP_BACKSPACE = 187
SDL_SCANCODE_KP_A = 188
SDL_SCANCODE_KP_B = 189
SDL_SCANCODE_KP_C = 190
SDL_SCANCODE_KP_D = 191
SDL_SCANCODE_KP_E = 192
SDL_SCANCODE_KP_F = 193
SDL_SCANCODE_KP_XOR = 194
SDL_SCANCODE_KP_POWER = 195
SDL_SCANCODE_KP_PERCENT = 196
SDL_SCANCODE_KP_LESS = 197
SDL_SCANCODE_KP_GREATER = 198
SDL_SCANCODE_KP_AMPERSAND = 199
SDL_SCANCODE_KP_DBLAMPERSAND = 200
SDL_SCANCODE_KP_VERTICALBAR = 201
SDL_SCANCODE_KP_DBLVERTICALBAR = 202
SDL_SCANCODE_KP_COLON = 203
SDL_SCANCODE_KP_HASH = 204
SDL_SCANCODE_KP_SPACE = 205
SDL_SCANCODE_KP_AT = 206
SDL_SCANCODE_KP_EXCLAM = 207
SDL_SCANCODE_KP_MEMSTORE = 208
SDL_SCANCODE_KP_MEMRECALL = 209
SDL_SCANCODE_KP_MEMCLEAR = 210
SDL_SCANCODE_KP_MEMADD = 211
SDL_SCANCODE_KP_MEMSUBTRACT = 212
SDL_SCANCODE_KP_MEMMULTIPLY = 213
SDL_SCANCODE_KP_MEMDIVIDE = 214
SDL_SCANCODE_KP_PLUSMINUS = 215
SDL_SCANCODE_KP_CLEAR = 216
SDL_SCANCODE_KP_CLEARENTRY = 217
SDL_SCANCODE_KP_BINARY = 218
SDL_SCANCODE_KP_OCTAL = 219
SDL_SCANCODE_KP_DECIMAL = 220
SDL_SCANCODE_KP_HEXADECIMAL = 221
SDL_SCANCODE_LCTRL = 224
SDL_SCANCODE_LSHIFT = 225
SDL_SCANCODE_LALT = 226
SDL_SCANCODE_LGUI = 227
SDL_SCANCODE_RCTRL = 228
SDL_SCANCODE_RSHIFT = 229
SDL_SCANCODE_RALT = 230
SDL_SCANCODE_RGUI = 231
SDL_SCANCODE_MODE = 257
SDL_SCANCODE_AUDIONEXT = 258
SDL_SCANCODE_AUDIOPREV = 259
SDL_SCANCODE_AUDIOSTOP = 260
SDL_SCANCODE_AUDIOPLAY = 261
SDL_SCANCODE_AUDIOMUTE = 262
SDL_SCANCODE_MEDIASELECT = 263
SDL_SCANCODE_WWW = 264
SDL_SCANCODE_MAIL = 265
SDL_SCANCODE_CALCULATOR = 266
SDL_SCANCODE_COMPUTER = 267
SDL_SCANCODE_AC_SEARCH = 268
SDL_SCANCODE_AC_HOME = 269
SDL_SCANCODE_AC_BACK = 270
SDL_SCANCODE_AC_FORWARD = 271
SDL_SCANCODE_AC_STOP = 272
SDL_SCANCODE_AC_REFRESH = 273
SDL_SCANCODE_AC_BOOKMARKS = 274
SDL_SCANCODE_BRIGHTNESSDOWN = 275
SDL_SCANCODE_BRIGHTNESSUP = 276
SDL_SCANCODE_DISPLAYSWITCH = 277
SDL_SCANCODE_KBDILLUMTOGGLE = 278
SDL_SCANCODE_KBDILLUMDOWN = 279
SDL_SCANCODE_KBDILLUMUP = 280
SDL_SCANCODE_EJECT = 281
SDL_SCANCODE_SLEEP = 282
SDL_SCANCODE_APP1 = 283
SDL_SCANCODE_APP2 = 284
SDL_SCANCODE_AUDIOREWIND = 285
SDL_SCANCODE_AUDIOFASTFORWARD = 286
SDL_SCANCODE_SOFTLEFT = 287
SDL_SCANCODE_SOFTRIGHT = 288
SDL_SCANCODE_CALL = 289
SDL_SCANCODE_ENDCALL = 290
SDL_SCANCODE_AMOUNT = 512

sdl_keyboard_down: # (r5 key)
  add ac r5 SDL_KEYBOARD_ADDRESS
  movb ac (ac)
  ret

sdl_font: # 7x7
  # ` `
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `!`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `"`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `#`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `$`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `%`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `&`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `'`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `(`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `)`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `*`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `+`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `,`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `-`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `.`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `/`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `0`
defb 0 1 1 1 1 0 0
defb 1 0 0 0 0 1 0
defb 1 0 0 0 0 1 0
defb 1 0 1 1 0 1 0
defb 1 0 0 0 0 1 0
defb 1 0 0 0 0 1 0
defb 0 1 1 1 1 0 0
  # `1`
defb 0 0 1 1 0 0 0
defb 0 1 0 1 0 0 0
defb 1 0 0 1 0 0 0
defb 0 0 0 1 0 0 0
defb 0 0 0 1 0 0 0
defb 0 0 0 1 0 0 0
defb 1 1 1 1 1 1 0
  # `2`
defb 0 1 1 1 1 0 0
defb 1 0 0 0 0 1 0
defb 0 0 0 0 1 0 0
defb 0 0 0 1 0 0 0
defb 0 0 1 0 0 0 0
defb 0 1 0 0 0 0 0
defb 1 1 1 1 1 1 0
  # `3`
defb 0 1 1 1 1 0 0
defb 1 0 0 0 0 1 0
defb 0 0 0 0 1 0 0
defb 0 0 0 1 1 0 0
defb 0 0 0 0 0 1 0
defb 1 0 0 0 0 1 0
defb 0 1 1 1 1 0 0
  # `4`
defb 0 0 0 0 1 0 0
defb 0 0 0 1 1 0 0
defb 0 0 1 0 1 0 0
defb 0 1 0 0 1 0 0
defb 1 1 1 1 1 1 0
defb 0 0 0 0 1 0 0
defb 0 0 0 0 1 0 0
  # `5`
defb 1 1 1 1 1 1 0
defb 1 0 0 0 0 0 0
defb 1 1 1 1 1 0 0
defb 0 0 0 0 0 1 0
defb 0 0 0 0 0 1 0
defb 1 0 0 0 0 1 0
defb 0 1 1 1 1 0 0
  # `6`
defb 0 0 1 1 1 0 0
defb 0 1 0 0 0 0 0
defb 1 0 0 0 0 0 0
defb 1 1 1 1 1 0 0
defb 1 0 0 0 0 1 0
defb 1 0 0 0 0 1 0
defb 0 1 1 1 1 0 0
  # `7`
defb 1 1 1 1 1 1 0
defb 0 0 0 0 0 1 0
defb 0 0 0 0 1 0 0
defb 0 0 0 1 0 0 0
defb 0 0 1 0 0 0 0
defb 0 1 0 0 0 0 0
defb 1 0 0 0 0 0 0
  # `8`
defb 0 1 1 1 1 0 0
defb 1 0 0 0 0 1 0
defb 1 0 0 0 0 1 0
defb 0 1 1 1 1 0 0
defb 1 0 0 0 0 1 0
defb 1 0 0 0 0 1 0
defb 0 1 1 1 1 0 0
  # `9`
defb 0 1 1 1 1 0 0
defb 1 0 0 0 0 1 0
defb 1 0 0 0 0 1 0
defb 0 1 1 1 1 1 0
defb 0 0 0 0 0 1 0
defb 0 0 0 0 1 0 0
defb 0 1 1 1 0 0 0
  # `:`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `;`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `<`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `=`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `>`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `?`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `@`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `A`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `B`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `C`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `D`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `E`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `F`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `G`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `H`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `I`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `J`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `K`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `L`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `M`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `N`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `O`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `P`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `Q`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `R`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `S`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `T`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `U`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `V`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `W`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `X`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `Y`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `Z`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `[`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `\`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `]`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `^`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `_`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # ```
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `a`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `b`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `c`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `d`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `e`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `f`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `g`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `h`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `i`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `j`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `k`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `l`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `m`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `n`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `o`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `p`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `q`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `r`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `s`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `t`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `u`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `v`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `w`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `x`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `y`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `z`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `{`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `|`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `}`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
  # `~`
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0
defb 0 0 0 0 0 0 0


sdl_render_glyph: # (r5 x, r6 y, r7 color, r8 glyph)
  pusha

  mov r1 0

sdl_render_glyph_loop:
  cmp r1 [7 * 7]
  jge sdl_render_glyph_end

  div r3 r1 7
  mov r2 ac

  mov r4 r1
  add r4 r4 r8
  movb r4 (r4)

  cmp r4 0
  jeq sdl_render_glyph_skip

  push r5
  push r6

  add r5 r5 r2
  add r6 r6 r3
  call sdl_render_point

  pop r6
  pop r5

sdl_render_glyph_skip:
  add r1 r1 1
  jmp sdl_render_glyph_loop

sdl_render_glyph_end:
  popa
  ret


sdl_render_char: # (r5 x, r6 y, r7 color, r8 char)
  pusha

  cmp r8 32
  jlt sdl_render_char_end

  cmp r8 126
  jgt sdl_render_char_end

  sub r8 r8 32
  mul r8 r8 [7 * 7]
  add r8 r8 sdl_font

  call sdl_render_glyph

sdl_render_char_end:
  popa
  ret


sdl_render_str: # (r5 x, r6 y, r7 color, r8 buffer)
  pusha

sdl_render_str_loop:
  movb r1 (r8)

  cmp r1 0
  jeq sdl_render_str_end

  push r8

  mov r8 r1
  call sdl_render_char

  pop r8

  add r5 r5 7
  add r8 r8 1
  jmp sdl_render_str_loop

sdl_render_str_end:
  popa
  ret

