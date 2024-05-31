#ifndef EDITOR_H
#define EDITOR_H

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <termios.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <unistd.h> // for read()
#include <stdarg.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/ioctl.h> // to interact with the terminal
#include <stdarg.h>



#define ABUF_INIT {NULL, 0}

// Data

typedef struct erow { // row
  int size;
  char *render;
  int rsize;
  char *chars;
} erow;
typedef struct {
    int cx, cy; // Cursor x and y position
    int rowoff; // Row offset
    int coloff; // Column offset
    int screenrows;
    int screencols;
    int numrows;
    int rx;
    erow *row;
    char *filename;
    int dirty;
    struct termios orig_termios;
    char statusmsg[80];
    time_t statusmsg_time;
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
void abAppend(struct abuf *ab, const char *s, int len); // output.c
void abFree(struct abuf *ab);// output.c
void enableRawMode(); // editor.c
void drawRows(struct abuf *ab); // output.c
void fileOpen(char *filename); // fileio.c
void setStatusMessage(const char *fmt, ...); // output.c
void insertRow(int at,char *s, size_t len); // fileio.c
void insertChar(int c); // row.c
void updateRow(erow *row);// row.c 
int rowCxToRx(erow *row, int cx); // row.c
char *rowsToString(int *buflen);  // fileio.c
void editorSave(); // fileio.c
void delChar(); // row.c
void insertNewline(); // row.c
char *promptUser(char *prompt, void(*callback)(char *, int)); // input.c
void find(); // find.c






extern editorConfig E;

#endif // EDITOR_H

