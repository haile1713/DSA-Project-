#include "editor.h"
enum Arrows {
    ARROW_LEFT = 1000, 
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
};

void findCallback(char * query, int key){
    static int last_match = -1; // -1 = nolast match
    static int direction = 1; // 1 forward -1 backward
    if (key == '\r' || key == '\x1b'){
        last_match = -1;
        direction = 1;
        return;
    } else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
         direction = 1;
    } else if (key == ARROW_LEFT || key == ARROW_UP) {
        direction = -1;
    } else {
        last_match = -1;
        direction = 1;
    }

    if (last_match == -1) direction = 1;
    int current = last_match;
    for (int i = 0; i < E.numrows; i++) {
        current += direction;
        if (current == -1) current = E.numrows - 1;
        else if (current == E.numrows) current = 0;
        erow *row = &E.row[current];
        // strstr() search for the substring query in the string row->render 
        char *match = strstr(row->render, query);
        if (match) {
            last_match = current;
            E.cy = current;
            E.cx = match - row->render;
            E.rowoff = E.numrows;
            break;
        }
    }
}

void find(){
    int saved_cx = E.cx;
    int saved_cy = E.cy;
    int saved_coloff = E.coloff;
    int saved_rowoff = E.rowoff;
    char *query = promptUser("Search: %s (ESC to cancel)", findCallback);
    if(query)
        free(query);
    else{
        E.cx = saved_cx;
        E.cy = saved_cy;
        E.coloff = saved_coloff;
        E.rowoff = saved_rowoff;
    }

}
