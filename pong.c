#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include "paddle.h"

// From ball.c
void init_ball_and_walls(int right_wall_col, int paddle_col);
void move_ball_once(int right_wall_col, int paddle_col);
void wrap_up();

// --------------
// Explanation:
//  1) We have top/bottom/left boundaries drawn plus a vertical wall behind the paddle.
//  2) We position the paddle "a little bit back" (not fully on the right edge).
//  3) The ball is spawned to the left of that paddle.
//  4) The game runs indefinitely until the user presses Q or q.
//  5) Use 'k' to move paddle up, 'j' to move paddle down.
// --------------

int main() {
    // Decide where the paddle goes and where the wall behind it goes
    int paddle_column   = 35;      // "pushed back only a little bit" from the left
    int behind_paddle_x = 36;      // wall behind the paddle

    // Initialize curses, draw walls, and place the ball
    init_ball_and_walls(behind_paddle_x, paddle_column);

    // Paddle: 5 rows tall, centered
    int top = (LINES / 2) - 2;
    int bot = (LINES / 2) + 2;
    paddle_init(top, bot, paddle_column, '#');

    // Make getch non-blocking so we can continuously update the ball
    nodelay(stdscr, TRUE);

    // Main loop
    while(1) {
        // Check user input
        int ch = getch();
        if(ch == 'Q' || ch == 'q') break;  // quit
        if(ch == 'j') paddle_down();
        if(ch == 'k') paddle_up();

        // Move the ball
        move_ball_once(behind_paddle_x, paddle_column);

        // Sleep a bit so it's not too fast
        usleep(30000);
    }

    // End curses
    wrap_up();
    return 0;
}
