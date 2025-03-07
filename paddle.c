#include <curses.h>
#include "paddle.h"

static struct pppaddle {
    int pad_top, pad_bot, pad_col;
    char pad_char;
} paddle;

static void draw_paddle() {
    for(int y = paddle.pad_top; y <= paddle.pad_bot; y++) {
        move(y, paddle.pad_col);
        addch(paddle.pad_char);
    }
    refresh();
}

void paddle_init(int top, int bot, int col, char ch) {
    paddle.pad_top = top;
    paddle.pad_bot = bot;
    paddle.pad_col = col;
    paddle.pad_char = ch;
    draw_paddle();
}

void paddle_up() {
    if(paddle.pad_top > 1) {
        for(int y = paddle.pad_top; y <= paddle.pad_bot; y++) {
            move(y, paddle.pad_col);
            addch(' ');
        }
        paddle.pad_top--;
        paddle.pad_bot--;
        draw_paddle();
    }
}

void paddle_down() {
    if(paddle.pad_bot < LINES-2) {
        for(int y = paddle.pad_top; y <= paddle.pad_bot; y++) {
            move(y, paddle.pad_col);
            addch(' ');
        }
        paddle.pad_top++;
        paddle.pad_bot++;
        draw_paddle();
    }
}

int paddle_contact(int y, int x) {
    return (x == paddle.pad_col && y >= paddle.pad_top && y <= paddle.pad_bot);
}
