/*** includes ***/
#include <ctype.h>
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

/*** data ***/
struct editorConfig{
    int screen_row;
    int screen_col;
    struct termios orig_termios; // save the original attributes of the terminal
};
struct editorConfig E;

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

char editor_read_key() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
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
void draw_tildas(write_buf *ab) {
    for(int y =0; y <E.screen_row; y++){
        ab_append( ab, "~", 1);
        ab_append( ab, "\x1b[K", 3); // erase a line
        if (y < E.screen_row - 1) { // don't put new line in the last row
        ab_append( ab, "\r\n", 2);
        }
    }
}

void refresh_the_screen(){
    write_buf ab = ABUF_INIT; 
    ab_append( &ab, "\x1b[H", 3); // go to the top
    ab_append( &ab, "\x1b[?25l", 6); // hide the cursor
    draw_tildas(&ab);
    ab_append( &ab, "\x1b[H", 3); // go to the top
    ab_append( &ab, "\x1b[?25l", 6); // hide the cursor
    write( STDIN_FILENO, ab.b, ab.len);
    ab_free(&ab);
}

/*** Input ***/
void when_key_pressed () {
    char c =  editor_read_key();
    switch (c) {
        case CTRL_KEY('q'):
            write(STDIN_FILENO,"\x1b[2J",4); // clear
            write(STDIN_FILENO,"\x1b[H",3);  // go to top
            exit(0);
            break;
        case CTRL_KEY('c'):
            refresh_the_screen();
            break;
        default:
            if (iscntrl(c)) {
              printf("%d\r\n", c);
            } else {
              printf("%d ('%c')\r\n", c, c);
            }
            
    }
}

/** inial **/

void editor_init(){
    if(get_window_size(&E.screen_row,&E.screen_col) == -1)
        die("can not get window size");
}
int main(){
    enable_row_mode();
    editor_init();
    refresh_the_screen();
    while (1) {
        when_key_pressed();
    }

    return 0;
}
