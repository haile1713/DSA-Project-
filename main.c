#include "editor.h"
editorConfig E; // for global access
int main() {
    enableRawMode();
    initEditor();
    while (1) {

        refreshScreen();
        processKeypress();
    }
    return 0;
}
