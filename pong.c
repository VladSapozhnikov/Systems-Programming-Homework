#include <curses.h>
#include <unistd.h>
#include "paddle.h"

// Functions from ball.c
void set_up(void);
void wrap_up(void);
void move_ball(void);
void start_game(void);
int is_finished(void);
int get_balls_left(void);

int main() {
    set_up();
    start_game();

    // Place paddle near the left side, about 5 columns in
    paddle_init(LINES/2 - 2, LINES/2 + 2, 5, '#');

    // Non-blocking input
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
