#ifndef PADDLE_H
#define PADDLE_H

void paddle_init(int top, int bot, int col, char ch);
void paddle_up();
void paddle_down();
int paddle_contact(int y, int x);

#endif
