#include <curses.h>
#include "paddle.h"

static struct {
    int top, bot, col;
    char ch;
} paddle;

// Draw the entire paddle at its current position
static void draw_paddle(void) {
    for(int y = paddle.top; y <= paddle.bot; y++){
        move(y, paddle.col);
        addch(paddle.ch);
    }
    refresh();
}

// Initialize paddle position and draw it
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
        // Erase current paddle
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
        // Erase current paddle
        for(int y = paddle.top; y <= paddle.bot; y++){
            move(y, paddle.col);
            addch(' ');
        }
        paddle.top++;
        paddle.bot++;
        draw_paddle();
    }
}

// Check if the ball at (y, x) is hitting the paddle
int paddle_contact(int y, int x) {
    return (x == paddle.col && y >= paddle.top && y <= paddle.bot);
}
