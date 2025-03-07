#include <curses.h>
#include <unistd.h>
#include "paddle.h"

// ball.c functions
void set_up(void);
void wrap_up(void);
void move_ball(void);
void start_game(void);
int  is_finished(void);
int  get_balls_left(void);

int main() {
    // Initialize everything
    set_up();
    start_game();

    // Push the racket back only a little bit near the right side:
    // We'll place it at col=COLS-5, vertical size ~5
    int paddle_col = COLS - 5;
    int top = (LINES / 2) - 2;
    int bot = (LINES / 2) + 2;
    paddle_init(top, bot, paddle_col, '#');

    // Non-blocking input
    nodelay(stdscr, TRUE);

    // Main loop
    while(!is_finished()) {
        int ch = getch();
        if(ch == 'Q' || ch == 'q') break;
        if(ch == 'j') paddle_down();
        if(ch == 'k') paddle_up();
        move_ball();
        usleep(30000);
        if(!get_balls_left()) break;
    }

    wrap_up();
    return 0;
}
