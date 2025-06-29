
attach "asm/std.asm"
attach "asm/stdx.asm"

PAD_SIZE      = 20.0
PAD_SIZE_HALF = [PAD_SIZE / 2]

PAD_MARGIN    = 2.0

PAD_SPEED     = 1.5

loop:
  sdl_begin

  # -------- UPDATE --------
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
    # -------- BALL MOVE --------
    mov r1 (b_x)
    mov r2 (b_y)

    mov r3 (b_vx)
    mov r4 (b_vy)

    cmp r2 PAD_MARGIN
    jgt skip_ball_neg_up

    std_neg r4

skip_ball_neg_up:
    cmp r2 [127.0 - PAD_MARGIN]
    jlt skip_ball_neg_down

    std_neg r4

skip_ball_neg_down:
    cmp r1 3.0
    jge skip_ball_neg_left

    mov r5 (p1_y)
    sub r6 r5 [PAD_SIZE_HALF + PAD_MARGIN]
    add r7 r5 [PAD_SIZE_HALF + PAD_MARGIN]

    cmp r2 r6
    jlt ball_reset_left

    cmp r2 r7
    jgt ball_reset_left

    jmp ball_neg_left

ball_reset_left:
    mov r1 64.0
    mov r2 64.0

    jmp skip_ball_position_update

ball_neg_left:
    std_neg r3

skip_ball_neg_left:
    cmp r1 [127.0 - 3.0]
    jle skip_ball_neg_right

    mov r5 (p2_y)
    sub r6 r5 [PAD_SIZE_HALF + PAD_MARGIN]
    add r7 r5 [PAD_SIZE_HALF + PAD_MARGIN]

    cmp r2 r6
    jlt ball_reset_right

    cmp r2 r7
    jgt ball_reset_right

    jmp ball_neg_right

ball_reset_right:
    mov r1 64.0
    mov r2 64.0

    jmp skip_ball_position_update

ball_neg_right:
    std_neg r3

skip_ball_neg_right:
    add r1 r1 r3
    add r2 r2 r4

skip_ball_position_update:
    mov (b_x) r1
    mov (b_y) r2

    mov (b_vx) r3
    mov (b_vy) r4

  # -------- RENDERING -------
    mov r5 64
    mov r6 0
    mov r7 127
    mov r8 0x111
    call sdl_render_vline


    # -------- PAD 1 --------
    mov r5 (p1_x)
    stdx_ftoi r5 r5

    mov r6 (p1_y)
    sub r6 r6 PAD_SIZE_HALF
    stdx_ftoi r6 r6

    mov r7 (p1_y)
    add r7 r7 PAD_SIZE_HALF
    stdx_ftoi r7 r7

    mov r8 0xF00

    call sdl_render_vline

    # -------- PAD 2 --------
    mov r5 (p2_x)
    stdx_ftoi r5 r5

    mov r6 (p2_y)
    sub r6 r6 PAD_SIZE_HALF
    stdx_ftoi r6 r6

    mov r7 (p2_y)
    add r7 r7 PAD_SIZE_HALF
    stdx_ftoi r7 r7

    mov r8 0x0F0

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

p1_x: def 2.0
p1_y: def 64.0

p2_x: def 125.0
p2_y: def 64.0

b_x: def 64.0
b_y: def 64.0

b_vx: def 0.66
b_vy: def 1.33

