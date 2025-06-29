
.PHONY: all vm-dbg vm-tty vm-sdl

CC := cc
CCFLAGS := -std=c11 -Wall -Wextra -Wpedantic

VM_OBJ := vm/vm.o
DBG_OBJ := frontend/dbg.o
TTY_OBJ := frontend/tty.o
SDL_OBJ := frontend/sdl.o

all: vm-dbg vm-tty vm-sdl

vm-dbg: $(VM_OBJ) $(DBG_OBJ)
	$(CC) $(CCFLAGS) $^ -o $@

vm-tty: $(VM_OBJ) $(TTY_OBJ)
	$(CC) $(CCFLAGS) $^ -o $@

vm-sdl: $(VM_OBJ) $(SDL_OBJ)
	$(CC) $(CCFLAGS) $^ -o $@ `sdl2-config --cflags --libs`

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm vm-dbg vm-tty vm-sdl $(VM_OBJ) $(DBG_OBJ) $(TTY_OBJ) $(SDL_OBJ)

