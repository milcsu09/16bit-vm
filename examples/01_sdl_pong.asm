
attach "asm/std.asm"
attach "asm/stdx.asm"

PAD_SIZE      = 20.0
PAD_SIZE_HALF = [PAD_SIZE / 2]

PAD_MARGIN    = 2.0

PAD_SPEED     = 0.8

entry:

loop:
  sdl_begin

  # -------- UPDATE --------
    # -------- BALL UPDATE --------
    mov r1 (b_x)
    mov r2 (b_y)

    mov r3 (b_vx)
    mov r4 (b_vy)

    movb r5 (b_dx)
    movb r6 (b_dy)

    # -------- BALL BOUNCE TOP --------
    cmp r2 PAD_MARGIN
    jgt skip_ball_neg_up

    std_lnot r6 r6

skip_ball_neg_up:
    cmp r2 [127.0 - PAD_MARGIN]
    jlt skip_ball_neg_down

    std_lnot r6 r6

skip_ball_neg_down:
    # -------- BALL BOUNCE SIDE --------
    cmp r1 3.0
    jge skip_ball_neg_left

    mov ac (p1_y)

    sub r7 ac [PAD_SIZE_HALF + PAD_MARGIN]
    add r8 ac [PAD_SIZE_HALF + PAD_MARGIN]

    cmp r2 r7
    jlt reset

    cmp r2 r8
    jgt reset

    sub ac r2 ac

    mov r6 0

    cmp ac 127.0
    jlt skip_ball_neg_left_normalize

    mov r6 1

    # (0xFFFF - ac) + 1 == 0x10000 - ac
    mov r8 0xFFFF
    sub ac r8 ac
    add ac ac 1

skip_ball_neg_left_normalize:
    shr ac ac 4
    mov r4 ac

    std_lnot r5 r5

skip_ball_neg_left:
    cmp r1 [127.0 - 3.0]
    jle skip_ball_neg_right

    mov ac (p2_y)

    sub r7 ac [PAD_SIZE_HALF + PAD_MARGIN]
    add r8 ac [PAD_SIZE_HALF + PAD_MARGIN]

    cmp r2 r7
    jlt reset

    cmp r2 r8
    jgt reset

    sub ac r2 ac

    mov r6 0

    cmp ac 127.0
    jlt skip_ball_neg_right_normalize

    mov r6 1

    # (0xFFFF - ac) + 1 == 0x10000 - ac
    mov r8 0xFFFF
    sub ac r8 ac
    add ac ac 1

skip_ball_neg_right_normalize:
    shr ac ac 4
    mov r4 ac

    std_lnot r5 r5

skip_ball_neg_right:
    # -------- BALL MOVE --------
    cmp r5 1
    jeq skip_ball_move_right

    add r1 r1 r3

    jmp skip_ball_move_left

skip_ball_move_right:
    sub r1 r1 r3

skip_ball_move_left:
    cmp r6 1
    jeq skip_ball_move_down

    add r2 r2 r4

    jmp skip_ball_move_up

skip_ball_move_down:
    sub r2 r2 r4

skip_ball_move_up:
  jmp skip_ball_reset

reset:
    cmp r1 64.0
    jlt skip_score_p1

    push r1
    mov r1 (p1_score)
    add r1 r1 1
    mov (p1_score) r1
    pop r1

    jmp skip_score_p2

skip_score_p1:
    push r1
    mov r1 (p2_score)
    add r1 r1 1
    mov (p2_score) r1
    pop r1

skip_score_p2:
    mov r1 64.0
    mov r2 64.0
    mov r4 0.0
    std_lnot r5 r5
    mov r6 0

    mov r7 (p1_y)
    mov r8 (p2_y)
    mov r7 64.0
    mov r8 64.0
    mov (p1_y) r7
    mov (p2_y) r8

skip_ball_reset:
    mov (b_x) r1
    mov (b_y) r2

    mov (b_vx) r3
    mov (b_vy) r4

    movb (b_dx) r5
    movb (b_dy) r6

    # -------- PAD 1 MOVE --------
    mov r5 SDL_SCANCODE_W
    call sdl_keyboard_down

    cmp ac 0
    jeq skip_update_pad1_up

    mov r1 (p1_y)
    sub ac r1 [PAD_SIZE_HALF + PAD_MARGIN]

    mov r2 PAD_SPEED
    cmp ac r2
    jge skip_update_pad1_up_clamp

    mov r2 ac

skip_update_pad1_up_clamp:
    sub r1 r1 r2
    mov (p1_y) r1

skip_update_pad1_up:
    mov r5 SDL_SCANCODE_S
    call sdl_keyboard_down

    cmp ac 0
    jeq skip_update_pad1_down

    mov r1 (p1_y)
    add ac r1 [PAD_SIZE_HALF + PAD_MARGIN]

    mov r2 PAD_SPEED

    mov r8 127.0
    sub r3 r8 r2

    cmp ac r3
    jle skip_update_pad1_down_clamp

    sub r2 r8 ac

skip_update_pad1_down_clamp:
    add r1 r1 r2
    mov (p1_y) r1

skip_update_pad1_down:
    # -------- PAD 2 MOVE --------
    mov r5 SDL_SCANCODE_UP
    call sdl_keyboard_down

    cmp ac 0
    jeq skip_update_pad2_up

    mov r1 (p2_y)
    sub ac r1 [PAD_SIZE_HALF + PAD_MARGIN]

    mov r2 PAD_SPEED
    cmp ac r2
    jge skip_update_pad2_up_clamp

    mov r2 ac

skip_update_pad2_up_clamp:
    sub r1 r1 r2
    mov (p2_y) r1

skip_update_pad2_up:
    mov r5 SDL_SCANCODE_DOWN
    call sdl_keyboard_down

    cmp ac 0
    jeq skip_update_pad2_down

    mov r1 (p2_y)
    add ac r1 [PAD_SIZE_HALF + PAD_MARGIN]

    mov r2 PAD_SPEED

    mov r8 127.0
    sub r3 r8 r2

    cmp ac r3
    jle skip_update_pad2_down_clamp

    sub r2 r8 ac

skip_update_pad2_down_clamp:
    add r1 r1 r2
    mov (p2_y) r1

skip_update_pad2_down:
    mov r5 (p1_score)
    mov r6 buffer_score
    call std_itoa

    mov r5 6
    mov r6 2
    mov r7 0x111
    mov r8 buffer_score
    call sdl_render_itext

    mov r5 (p2_score)
    mov r6 buffer_score
    call std_itoa

    mov r5 [64 + 6]
    mov r6 2
    mov r7 0x111
    mov r8 buffer_score
    call sdl_render_itext


  # -------- RENDERING -------
    mov r5 64
    mov r6 0
    mov r7 127
    mov r8 0x111
    call sdl_render_vline

    # -------- PAD 1 --------
    mov r5 2

    mov r6 (p1_y)
    sub r6 r6 PAD_SIZE_HALF
    stdx_ftoi r6 r6

    mov r7 (p1_y)
    add r7 r7 PAD_SIZE_HALF
    stdx_ftoi r7 r7

    mov r8 0xF24

    call sdl_render_vline

    # -------- PAD 2 --------
    mov r5 [127 - 2]

    mov r6 (p2_y)
    sub r6 r6 PAD_SIZE_HALF
    stdx_ftoi r6 r6

    mov r7 (p2_y)
    add r7 r7 PAD_SIZE_HALF
    stdx_ftoi r7 r7

    mov r8 0x29F

    call sdl_render_vline

    # -------- BALL --------
    mov r5 (b_x)
    stdx_ftoi r5 r5

    mov r6 (b_y)
    stdx_ftoi r6 r6

    mov r7 0xFFF

    call sdl_render_point

  sdl_end

  jmp loop

p1_y: def 64.0
p2_y: def 64.0

p1_score: def 0
p2_score: def 0

buffer_score: resb 5

b_x: def 64.0
b_y: def 64.0

b_vx: def 0.625
b_vy: def 0.0

b_dx: defb 0
b_dy: defb 0

