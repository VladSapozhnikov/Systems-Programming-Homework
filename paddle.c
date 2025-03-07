#include <curses.h>
#include "paddle.h"

static struct {
    int top, bot, col;
    char ch;
} paddle;

static void draw_paddle(void) {
    for(int y = paddle.top; y <= paddle.bot; y++){
        move(y, paddle.col);
        addch(paddle.ch);
    }
    refresh();
}

void paddle_init(int top, int bot, int col, char ch) {
    paddle.top = top;
    paddle.bot = bot;
    paddle.col = col;
    paddle.ch  = ch;
    draw_paddle();
}

void paddle_up() {
    if(paddle.top > 1) {
        for(int y = paddle.top; y <= paddle.bot; y++){
            move(y, paddle.col);
            addch(' ');
        }
        paddle.top--;
        paddle.bot--;
        draw_paddle();
    }
}

void paddle_down() {
    if(paddle.bot < LINES - 2) {
        for(int y = paddle.top; y <= paddle.bot; y++){
            move(y, paddle.col);
            addch(' ');
        }
        paddle.top++;
        paddle.bot++;
        draw_paddle();
    }
}

int paddle_contact(int y, int x) {
    return (x == paddle.col && y >= paddle.top && y <= paddle.bot);
}
