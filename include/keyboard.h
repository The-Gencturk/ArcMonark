#ifndef KEYBOARD_H
#define KEYBOARD_H

void keyboard_init(void);
char keyboard_getchar(void);
int  keyboard_haskey(void);          // non-bloklayici: bekleyen tus var mi?
void read_line(char* buf, int max);

#endif