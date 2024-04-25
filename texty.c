#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

struct termios orig_termios; // save the original attributes of the terminal
void disableRowMode() {
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&orig_termios);
}

// explane time 
// ECHO is a bunch of 0s and 1 making is show charactors written in the input and make make a logical not to invert and logical and to set every thing true but the echo functionality ICANON -> sets chronological typeing on making it read each line at a time making that false makes it read each byte by byte 
// ISIG diables <C-c> and <C-z> which both sends SIGINT signal and SIGTSTP signal
// IXON disables <C-s> and <C-q> that stop on and off  input to the terminal
// IEXTEN disables <C-v> 
void enableRowMode() {
    tcgetattr(STDIN_FILENO,&orig_termios); // get attributes from terminal
    atexit(disableRowMode);

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | IXON | ICRNL);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); 
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw); // set the termianl attributes

}

int main(){
    enableRowMode();
    while (1) {
        char c = '\0';
        read(STDIN_FILENO, &c, 1);
        if (iscntrl(c)){
             printf("%d\r\n", c);
        }
        else{
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') break;
    };
    return 0;
}

