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

// Move paddle up by one row if possible
void paddle_up() {
    if(paddle.top > 1) {
        // Erase old paddle
        for(int y = paddle.top; y <= paddle.bot; y++){
            move(y, paddle.col);
            addch(' ');
        }
        paddle.top--;
        paddle.bot--;
        draw_paddle();
    }
}

// Move paddle down by one row if possible
void paddle_down() {
    if(paddle.bot < LINES - 2) {
        // Erase old paddle
        for(int y = paddle.top; y <= paddle.bot; y++){
            move(y, paddle.col);
            addch(' ');
        }
        paddle.top++;
        paddle.bot++;
        draw_paddle();
    }
}

// True if the (y, x) point is occupied by the paddle
int paddle_contact(int y, int x) {
    return (x == paddle.col && y >= paddle.top && y <= paddle.bot);
}
