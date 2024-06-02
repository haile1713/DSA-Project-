#include "editor.h"
#include <ctype.h>
#include <time.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define NUM_QUIT 1;

// auto set other incrementally 1001
enum Arrows {
    BACKSPACE = 127,
    ARROW_LEFT = 1000, 
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    PAGE_UP,
    PAGE_DOWN,
    HOME_KEY,
    END_KEY,
    DEL_KEY,
};

void moveCursor(enum Arrows key) {
  erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
  switch (key) {
    case BACKSPACE:
      break;
    case ARROW_LEFT:
      if (E.cx > 0) {
         E.cx--;
      } else if (E.cy > 0) {
         E.cy--;
         E.cx = E.row[E.cy].size;
      }
      break;
    case ARROW_RIGHT:
        if (row && E.cx < row->size) {
            E.cx++;
        } else if (row && E.cx == row->size) {
            E.cy++;
            E.cx = 0;
        }
      break;

    case ARROW_UP:
        if(E.cy >0){
            E.cy--;
            if(E.cx > E.row[E.cy].size) // snap to the end of the line
                E.cx = E.row[E.cy].size; 
        }

      break;
    case ARROW_DOWN:
         if(E.cy < E.numrows) {
            E.cy++;
            if(E.cx > E.row[E.cy].size)
                E.cx = E.row[E.cy].size; 
         }
      break;
    case PAGE_DOWN:
        if(E.cy <E.screenrows)
            E.cy = E.screenrows;

      break;
    case PAGE_UP:
        if(E.cy >0)
            E.cy=E.rowoff;
      break;
    case HOME_KEY:
         E.cx=E.coloff;
      break;
    case END_KEY:
         E.cx = row->size; 
      break;
    case DEL_KEY:
      break;
  }
}


int readKey() {
    int nread;
    char c;
    char esc = '\x1b';
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) { // check if there is error in reading the keyboard input
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    if (c == esc) {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b'; // check if the first value is available
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b'; // check if the second value is available


        if (seq[0] == '[') {

             if (seq[1] >= '0' && seq[1] <= '9') { // check if is is a number
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b'; // check
                if (seq[2] == '~') { // the last
                  switch (seq[1]) {
                    case '1': return HOME_KEY;  // esc[1~
                    case '3': return DEL_KEY;   // esc[3~
                    case '4': return END_KEY;   // esc[2~
                    case '5': return PAGE_UP;   // esc[5~
                    case '6': return PAGE_DOWN; // esc[6~
                    case '7': return HOME_KEY;  // esc[7~
                    case '8': return END_KEY;   // esc[8~

                  }
                }
             } else {
                switch (seq[1]) {
                    case 'A': return ARROW_UP;    // esc[A
                    case 'B': return ARROW_DOWN;  // esc[B
                    case 'C': return ARROW_RIGHT; // esc[C
                    case 'D': return ARROW_LEFT;  // esc[D
                    case 'H': return HOME_KEY;    // esc[C
                    case 'F': return END_KEY;     // esc[D

                }
            }
            
        } else if (seq[0] == 'O') {
                switch (seq[1]) {
                    case 'H': return HOME_KEY;  //escOH
                    case 'F': return END_KEY;   //escOF
                }
        }
        return '\x1b';
    } else {
        return c;
    }
}


void processKeypress() {
    int c = readKey();
    static int quit_times = NUM_QUIT;
    switch (c) {
        case '\r': // Enter key
            insertNewline();
        break;
        case CTRL_KEY('q'):
            if(E.dirty && quit_times > 0){
                setStatusMessage("WARNING!!! File has unsaved changes. Press Ctrl-Q %d more times to quit", quit_times);
                quit_times--;
                return;
            }
            die("ቻው ቻው ");
            break;
        case CTRL_KEY('s'):
            editorSave();
            break;
        case CTRL_KEY('f'):
            find();
        break;

        case HOME_KEY:
        case END_KEY:
        case PAGE_UP:
        case PAGE_DOWN:
        case ARROW_UP  :
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            moveCursor(c);
            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            if (c == DEL_KEY) moveCursor(ARROW_RIGHT);
            delChar();
        break;

        case CTRL_KEY('l'):
        case '\x1b':
        break;

        default:
            insertChar(c);
            break;
    }
     quit_times = NUM_QUIT; // reset back 
}

char *promptUser(char *prompt, void (*callback)(char *,int)) {
    size_t  bufsize = 128; 
    char *buf = malloc(bufsize);
    size_t buflen = 0;
    buf[0] = '\0';
    while(1){
        setStatusMessage(prompt, buf);
        refreshScreen();
        int c = readKey();
        if(c == '\x1b'){ // if esc is pressed
            setStatusMessage("");
            if(callback) callback(buf, c);
            free(buf);
            return NULL;
        }
        else if(c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE){
            if(buflen != 0) buf[--buflen] = '\0';

        }
        else if(c == '\r'){ // if enter is pressed
            if(buflen != 0){
                setStatusMessage("");// clean the status bar
                if(callback) callback(buf, c);
                return buf;
            }
        }else if(!iscntrl(c) && c < 128){
            if(buflen == bufsize - 1){ // if you are only left with one space add more space
                bufsize *= 2;
                buf = realloc(buf, bufsize);
            }
            buf[buflen++] = c; // set the buffer to the character
            buf[buflen] = '\0';
        } 
        if(callback) callback(buf, c);

    }
}
