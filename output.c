#include "editor.h"


void drawRows(struct abuf *ab) {
    for (int y = 0; y < E.screenrows; y++) {
        if (y == E.screenrows / 3) {
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome), "Texty for DSA -v %s", "0.01");
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
        abAppend(ab, "\x1b[K", 3); // clear the line 
        if (y < E.screenrows - 1) {
            abAppend(ab, "\r\n", 2); // move to the next line
        }
    }
}
void refreshScreen() {
    struct abuf ab = ABUF_INIT;

    // Hide the cursor
    abAppend(&ab, "\x1b[?25l", 6);

    // Move the cursor to the top-left corner
    abAppend(&ab, "\x1b[H", 3);

    // Draw rows to the buffer
    drawRows(&ab);
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
    abAppend(&ab, buf, strlen(buf));

    // Move the cursor to the top-left corner again
    abAppend(&ab, "\x1b[H", 3);

    // Show the cursor
    abAppend(&ab, "\x1b[?25h", 6);

    // Write the buffer to the terminal
    write(STDOUT_FILENO, ab.b, ab.len);

    // Free the buffer
    abFree(&ab);
}
