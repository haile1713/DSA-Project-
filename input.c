#include "editor.h"
#define CTRL_KEY(k) ((k) & 0x1f)


void moveCursor(char key) {
  switch (key) {
    case 'h':
      E.cx--;
      break;
    case 'l':
      E.cx++;
      break;
    case 'k':
      E.cy--;
      break;
    case 'j':
      E.cy++;
      break;
  }
}
void processKeypress() {
    char c = '\0';
    read(STDIN_FILENO, &c, 1);

    switch (c) {
        case CTRL_KEY('q'):
            die("bye bye!");
            break;
        case 'j':
        case 'k':
        case 'h':
        case 'l':
            moveCursor(c);
            break;
    }
}

