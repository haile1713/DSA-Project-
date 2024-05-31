#include "editor.h"
#include <stdio.h>
const int TAB_STOP = 4;

void updateRow(erow *row) {
    free(row->render);
    row->render = malloc(row->size + 1);
    int idx = 0;
    for (int i = 0; i < row->size; i++) {
        row->render[idx++] = row->chars[i];
    }
    row->render[idx] = '\0';
    row->rsize = idx;
}
int rowCxToRx(erow *row, int cx) {
  int rx = 0;
  int j;
  for (j = 0; j < cx; j++) {
    if (row->chars[j] == '\t')
      rx += (TAB_STOP - 1) - (rx % TAB_STOP);
    rx++;
  }
  return rx;
}

void insertRow(int at ,char *s, size_t len) {
    if (at < 0 || at > E.numrows) return;

    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
    memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));

    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';
    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    updateRow(&E.row[at]);

    E.numrows++;
    E.dirty++;
}
void insertNewline() {
  if (E.cx == 0) {
    insertRow(E.cy, "", 0);
  } else { // we have to split the line into 2
    erow *row = &E.row[E.cy];
    insertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx); // from curr to end put on a new line
    row = &E.row[E.cy]; // reassign row if it has been reallocated
    row->size = E.cx; 
    row->chars[row->size] = '\0';
    updateRow(row);
  }
  E.cy++;
  E.cx = 0;
}
void rowInsertChar(erow *row, int at, int c) {
    if (at < 0 || at > row->size) at = row->size; // if it is  out of bound
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->size++;
    row->chars[at] = c;
    updateRow(row);
    E.dirty++;

}


void insertChar(int c) {
    if (E.cy == E.numrows) {
        insertRow(E.numrows,"", 0);
    }
        rowInsertChar(&E.row[E.cy], E.cx, c);
    E.cx++;
}

char *rowsToString(int *buflen) {
    int totlen = 0;
    for (int i = 0; i < E.numrows; i++) // loop through the rows
        totlen += E.row[i].size + 1; // add the size of the row and 1 for the newline character

    *buflen = totlen;
    char *buf = malloc(totlen);
    char *p = buf;
    for (int j = 0; j < E.numrows; j++) {
        memcpy(p, E.row[j].chars, E.row[j].size); // copy the row to the buffer
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    return buf; // the caller should free the buffer
}

void freeRow(erow *row) {
  free(row->render);
  free(row->chars);
}
void delRow(int at) {
  if (at < 0 || at >= E.numrows) return;
  freeRow(&E.row[at]);
  memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
  E.numrows--;
  E.dirty++;
}
void rowAppendString(erow *row, char *s, size_t len) {
  row->chars = realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = '\0';
  updateRow(row);
  E.dirty++;
}


void rowDelChar(erow *row, int at) {
  if (at < 0 || at >= row->size) return;
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  updateRow(row);
  E.dirty++;
}
void delChar() { 
  if (E.cy == E.numrows) return;
  if (E.cx == 0 && E.cy == 0) return;
  erow *row = &E.row[E.cy];
  if (E.cx > 0) {
    rowDelChar(row, E.cx - 1);
    E.cx--;
  }else{ // at the beginning of the line
    E.cx = E.row[E.cy - 1].size;
    rowAppendString(&E.row[E.cy - 1], row->chars, row->size); // append a string to the previous row
    delRow(E.cy); // delete current row 
    E.cy--;
  }
}
