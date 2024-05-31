#include "editor.h"
editorConfig E; // for global access
int main(int argc, char *argv[]) {
    enableRawMode();
    initEditor();
    setStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit  | Ctrl-F = find");

    if (argc >= 2) {
      fileOpen(argv[1]);
    }
    while (1) {

        refreshScreen();
        processKeypress();
    }
    return 0;
}
