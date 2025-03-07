#include <curses.h>
#include <unistd.h>
#include "paddle.h"

// From ball.c
void set_up();
void wrap_up();
void move_ball();
void start_game();
int is_finished();
int get_balls_left();

int main() {
    set_up();
    start_game();
    // Paddle near the right edge, 5 rows tall
    paddle_init(LINES/2 - 2, LINES/2 + 2, COLS - 2, '#');
    nodelay(stdscr, TRUE);

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
