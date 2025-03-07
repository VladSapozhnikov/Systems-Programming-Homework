#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include "paddle.h"

// From ball.c
void init_ball_and_walls(int right_wall_col, int paddle_col);
void move_ball_once(int right_wall_col, int paddle_col);
void wrap_up(void);
void redraw_boundaries_and_paddle(int right_wall_col, int paddle_col);

int main() {
    // Paddle slightly back from the left, plus a wall behind it
    int paddle_column   = 30;  // user said "pushed back only a little bit"
    int behind_paddle_x = 31;  // vertical wall behind the paddle

    // Init curses, walls, and ball
    init_ball_and_walls(behind_paddle_x, paddle_column);

    // Create a paddle: 5 rows tall, near the vertical center
    int top = (LINES / 2) - 2;
    int bot = (LINES / 2) + 2;
    paddle_init(top, bot, paddle_column, '#');

    // Non-blocking input
    nodelay(stdscr, TRUE);

    // Main loop: run until user presses Q
    while(1) {
        int ch = getch();
        if(ch == 'Q' || ch == 'q') break;
        if(ch == 'k') paddle_up();
        if(ch == 'j') paddle_down();

        // After potential paddle movement, re-draw boundaries & the paddle
        redraw_boundaries_and_paddle(behind_paddle_x, paddle_column);

        // Move the ball
        move_ball_once(behind_paddle_x, paddle_column);

        usleep(30000);
    }

    wrap_up();
    return 0;
}
