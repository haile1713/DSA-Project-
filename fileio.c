#include "editor.h"
#include <fcntl.h> 
void fileOpen(char *filename) {
    free(E.filename);
    E.filename = strdup(filename);
    FILE *fp = fopen(filename, "r");
    if (!fp){
        setStatusMessage("Can't open file! I/O error: %s", strerror(errno));
        return;
    }
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' ||
                               line[linelen - 1] == '\r'))
          linelen--;
        insertRow(E.numrows,line, linelen);
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
}
void editorSave() {
  if (E.filename == NULL){
        E.filename = promptUser("Save as: %s (ESC to cancel)",NULL);

        if (E.filename == NULL) {
            setStatusMessage("Save aborted");
            return;
        }
    }
  int len;
  char *buf = rowsToString(&len);
   //  O_CREAT is a flag for telling it a newly created file
   //  O_RDWR is for granting permition for read and write

  int fd = open(E.filename, O_RDWR | O_CREAT, 0644); // open is from fcntl.h   
      if (fd != -1) {
        if (ftruncate(fd, len) != -1) {   //sets the file size to the specified length or trancautes 
          if (write(fd, buf, len) == len) {
              close(fd);
              free(buf);
              E.dirty =0;
              setStatusMessage("%d bytes written to disk", len);
              return;
          }
        }
        close(fd);
     }
    free(buf);
    setStatusMessage("Can't save! I/O error: %s", strerror(errno));
}
