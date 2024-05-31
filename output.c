#include "editor.h"
#include <stdio.h>

void scroll() {
  E.rx = E.cx;
  if (E.cy < E.numrows) {
    E.rx = rowCxToRx(&E.row[E.cy], E.cx);
  }

  if (E.cy < E.rowoff) { // checks if the cursor is above the visible window
    E.rowoff = E.cy;
  }
  if (E.cy >= E.rowoff + E.screenrows) { // checks if the cursor is below the visible window
    E.rowoff = E.cy - E.screenrows + 1;
  }
  if (E.rx < E.coloff) {
    E.coloff = E.rx;
  }
  if (E.rx >= E.coloff + E.screencols) {
    E.coloff = E.rx - E.screencols + 1;
  }
}
void drawMessageBar(struct abuf *ab) {
  abAppend(ab, "\x1b[K", 3);
  int msglen = strlen(E.statusmsg);
  if (msglen > E.screencols) msglen = E.screencols;
  if (msglen && time(NULL) - E.statusmsg_time < 5)
    abAppend(ab, E.statusmsg, msglen);
}



void drawRows(struct abuf *ab) {
    for (int y = 0; y < E.screenrows; y++) {
        int filerow = y + E.rowoff;
        if(filerow >= E.numrows){ // if the row is beyond the end of the file
            //
            if (E.numrows == 0 && y == E.screenrows / 3) { // write the text about a third of the way 
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome), 
                                "ጽሁፍ መ ጻፍያ  for DSA -v %s", "0.01");
                if (welcomelen > E.screencols) welcomelen = E.screencols;
                int padding = (E.screencols - welcomelen) / 2;
                if (padding) {
                    abAppend(ab, "~", 1);
                    padding--;
                }
                while (padding--) abAppend(ab, " ", 1);
                abAppend(ab, welcome, welcomelen);
            } else {
                abAppend(ab, "~", 1); 
                }
        }else {
               int len = E.row[filerow].size - E.coloff; // length of the row
               if (len < 0) len = 0;
               if (len > E.screencols) len = E.screencols; 
               abAppend(ab, &E.row[filerow].chars[E.coloff], len);
        }
        abAppend(ab, "\x1b[K", 3); // clear the line 
        abAppend(ab, "\r\n", 2); // move to the next line
    }
}
void setStatusMessage(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
  va_end(ap);
  E.statusmsg_time = time(NULL);
}

void drawStatusBar(struct abuf *ab) {
    abAppend(ab, "\x1b[7m", 4); // invert the colors
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s", 
      E.filename ? E.filename : "[No Name]", E.numrows,  // write the filename and number of lines
      E.dirty ? "(modified)" : "");
    int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d",
    E.cy + 1, E.numrows);
    if (len > E.screencols) len = E.screencols;

    abAppend(ab, status, len); // write the status bar
    
    while (len < E.screencols) {
        if (E.screencols - len == rlen) {
          abAppend(ab, rstatus, rlen);
          break;
        } else {
        abAppend(ab, " ", 1);
        len++;
        }
    }
    abAppend(ab, "\x1b[m", 3);
    abAppend(ab, "\r\n", 2);
}
void refreshScreen() {
    scroll();
    struct abuf ab = ABUF_INIT;
    abAppend(&ab, "\x1b[?25l", 6); // Hide the cursor
    abAppend(&ab, "\x1b[H", 3);    // Move the cursor to the top-left corner

    drawRows(&ab);  // Draw rows to the buffer
    drawStatusBar(&ab);
    drawMessageBar(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, (E.rx - E.coloff) + 1);
    abAppend(&ab, buf, strlen(buf));  // Correctly position the cursor

    abAppend(&ab, "\x1b[?25h", 6);  // Show the cursor

    write(STDOUT_FILENO, ab.b, ab.len);  // Write the buffer to the terminal

    abFree(&ab);  // Free the buffer
}
