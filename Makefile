
.PHONY: all dbg

all: dbg

dbg:
	gcc -std=c11 -Wall -Wextra -Wpedantic $(wildcard vm/*.c) $(wildcard dbg/*.c) -o vm-dbg

