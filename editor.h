#ifndef EDITOR_H
#define EDITOR_H

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
// #define _GNU_SOURCE

#include <termios.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#define ABUF_INIT {NULL, 0}

// Data

typedef struct {
    int cx, cy; // Cursor x and y position
    int rowoff; // Row offset
    int coloff; // Column offset
    int screenrows;
    int screencols;
    struct termios orig_termios;
} editorConfig;

struct abuf { // storing the buffer 
  char *b;
  int len;
};

// Function declarations
void initEditor(); // editor.c
void refreshScreen(); // output.c
void processKeypress(); // input.c
void die(const char *s); // editor.c
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);
void enableRawMode(); // editor.c
void DrawRows(struct abuf *ab); // output.c

extern editorConfig E;

#endif // EDITOR_H

