/*** includes ***/

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)
#define TEXTY_VERSION  "0.0.1"
enum editorKey {
  ARROW_LEFT = 1000,
  ARROW_RIGHT ,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};


/*** data ***/
typedef struct erow {
  int size;
  char *chars;
} erow;

struct editor_config{
    int cursorX, cursorY;
    int rowoff;
    int coloff;
    int screen_row;
    int screen_col;
    int numrows;
    erow *row;
    struct termios orig_termios; // save the original attributes of the terminal
};
struct editor_config E;

/*** terminal ***/
void die(const char *s){
    write(STDOUT_FILENO, "\x1b[2J", 4); // clear
    write(STDOUT_FILENO, "\x1b[H", 3); // go to top
    perror(s);
    exit(1);
}
void disableRowMode() {
    if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&E.orig_termios)== -1)
        die("tcsetattr");
}

void enable_row_mode() {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
    atexit(disableRowMode);

    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | IXON | ICRNL);
    raw.c_oflag &= ~(OPOST); // remove translation from \n to \r\n
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); 
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int editor_read_key() {
      int nread;
      char c;
      while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
      }
      if (c == '\x1b') {
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~') {
                  switch (seq[1]) {
                    case '1': return HOME_KEY;
                    case '4': return END_KEY;
                    case '3': return DEL_KEY;
                    case '5': return PAGE_UP;
                    case '6': return PAGE_DOWN;
                    case '7': return HOME_KEY;
                    case '8': return END_KEY;
                  }
                }
            }
            else{
              switch (seq[1]) {
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
                }
            }
        }
        else if (seq[0] == 'O') {
          switch (seq[1]) {
            case 'H': return HOME_KEY;
            case 'F': return END_KEY;
          }
        }

        return '\x1b';
    } else {
        return c;
    }
}
int get_cursor_position(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
    return 0;
}

int get_window_size(int *rows, int *cols) {
  struct winsize ws;
  if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1; // get the position bottom right
    return get_cursor_position(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}
/** row operations**/
void editor_append_row(char *s , size_t len){
  E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));

  int now_at = E.numrows;
  
  E.row[now_at].size = len;
  E.row[now_at].chars  = malloc(len +1);

  memcpy(E.row[now_at].chars, s, len);
  E.row[now_at].chars[len] = '\0';
  E.numrows ++;
}
/** file I/O **/ 
void editor_open(char *filename){
  FILE *fp = fopen(filename, "r");
  if(!fp)die("can not open file");


  char *line  = NULL;
  size_t linecap = 0;
  ssize_t linelen = 13; 
  if(linelen != -1){
    while((linelen = getline(&line, &linecap, fp)) != -1){
      while(linelen > 0 && (line[linelen-1] == '\n' || line[linelen-1]  == '\r'))
        linelen--;
      editor_append_row(line, linelen);
    }
  }
  free(line);
  fclose(fp);
}

/**Append buffer**/


struct abuf {
    char *b;
    int len;
};
typedef struct abuf write_buf ;
#define ABUF_INIT {NULL, 0}

void ab_append(write_buf *ab, const char *s, int len){
    char *new = realloc(ab->b, ab -> len +  len);
    if (new == NULL) return;
    memcpy(&new[ab->len],s,len); // cpy the string string form we stoped
    ab->b = new; // update the string to the new string
    ab -> len += len;
}
void ab_free(write_buf *ab){
    free(ab->b);
}

/** Output **/
void editor_scroll(){
  if(E.cursorY < E.rowoff){
    E.rowoff = E.cursorY;
  }
  if(E.cursorY > E.rowoff + E.screen_row){
    E.rowoff = E.cursorY - E.screen_row + 1;
  }

  if (E.cursorX < E.coloff) {
    E.coloff = E.cursorX;
  }
  if (E.cursorX >= E.coloff + E.screen_col) {
    E.coloff = E.cursorX - E.screen_col + 1;
  }
}
void editor_draw_row(write_buf *ab) {
    for(int y =0; y <E.screen_row; y++){
      int filerow = y + E.rowoff;
      if(filerow >= E.numrows){
        if(E.numrows == 0 && y == E.screen_row/3){
          char welcome[80];
          int welcomelen = snprintf(welcome, sizeof(welcome),"Texty -- version %s", TEXTY_VERSION);
          if (welcomelen > E.screen_col) welcomelen = E.screen_col;

          int padding = (E.screen_col - welcomelen) / 2;
          if (padding) {
            ab_append(ab, "~", 1);
            padding--;
          }
          while (padding--) ab_append(ab, " ", 1);
          ab_append(ab,welcome, welcomelen);

        }else{
            ab_append( ab, "~", 1);
           }
      }
      else{
          int len = E.row[filerow].size - E.coloff;
          if (len<0) len = 0;
          if (len > E.screen_col) len = E.screen_col;
          ab_append(ab, &E.row[filerow].chars[E.coloff], len);
      }
      
        ab_append( ab, "\x1b[K", 3); // erase a line
        if (y < E.screen_row - 1) { // don't put new line in the last row
          ab_append( ab, "\r\n", 2);
        }

    }
}

void refresh_the_screen(){
    editor_scroll();

    write_buf ab = ABUF_INIT; 


    ab_append( &ab, "\x1b[H", 3); // go to the top
    // ab_append( &ab, "\x1b[?25l", 6); // hide the cursor
    //
    editor_draw_row(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cursorY - E.rowoff) + 1, (E.cursorX -E.coloff) + 1); // move the cursor to cy and cx
    ab_append(&ab, buf, strlen(buf));

    // ab_append( &ab, "\x1b[?25l", 6); // hide the cursor
    write( STDIN_FILENO, ab.b, ab.len);
    ab_free(&ab);
}

/*** Input ***/
void move_cursor(int key) {
  switch (key) {
    case ARROW_LEFT:
      if (E.cursorX != 0) {
        E.cursorX--;
       } else if (E.cursorY > 0) {
        E.cursorY--;
        E.cursorX = E.row[E.cursorY].size;
      }
      break;
    case ARROW_RIGHT:
        E.cursorX++;
      break;
    case ARROW_UP:
      if (E.cursorY != 0) {
        E.cursorY--;
      }
      break;
    case ARROW_DOWN:
      if (E.cursorY <E.numrows) {
        E.cursorY++;
      }
      break;
  }
}
void when_key_pressed () {
    int c =  editor_read_key();
    switch (c) {
        case CTRL_KEY('q'):
            write(STDIN_FILENO,"\x1b[2J",4); // clear
            write(STDIN_FILENO,"\x1b[H",3);  // go to top
            exit(0);
            break;
        case CTRL_KEY('c'):
            refresh_the_screen();
            break;
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            move_cursor(c);
            break;
        case HOME_KEY:
            E.cursorX = 0;
        break;
        case END_KEY:
            E.cursorX = E.screen_col - 1;
        break;
        case PAGE_UP:
        case PAGE_DOWN:
          {
            int times = E.screen_row;
            while (times--)
              move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
          }
          break;

        // default:
        //     if (iscntrl(c)) {
        //       printf("%d\r\n", c);
        //     } else {
        //       printf("%d ('%c')\r\n", c, c);
        //     }
    }
}

/** initailized **/
void editor_init(){
    E.cursorX = 0; // start the cursor pos from top left
    E.cursorY = 0;
    E.numrows = 0;
    E.row = NULL;
    E.rowoff = 0;
    E.coloff = 0;
    if(get_window_size(&E.screen_row,&E.screen_col) == -1)
        die("can not get window size");
}
int main(int argc, char * argv[]){
    enable_row_mode();
    editor_init();
    if(argc >= 2){
      editor_open(argv[1]);
   }
    while (1) {
        refresh_the_screen();
        when_key_pressed();
    }

    return 0;
}
