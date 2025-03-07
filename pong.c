#include <curses.h>
#include <unistd.h>
#include "paddle.h"

// From ball.c
void set_up(void);
void wrap_up(void);
void move_ball(void);
void start_game(void);
int  is_finished(void);
int  get_balls_left(void);

int main() {
    set_up();
    start_game();

    // Paddle at about 3/4 of screen width => "halfway back" from the far-right
    int paddle_col = (3 * COLS) / 4;
    // Paddle 5 rows tall around the vertical 
