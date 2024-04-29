CFLAGS =-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags sdl2`
LIBS = `pkg-config --libs sdl2`
texty: texty.c
	$(CC) $(CFLAGS) -o texty texty.c $(LIBS)
