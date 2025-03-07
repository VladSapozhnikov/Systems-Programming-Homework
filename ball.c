#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include "paddle.h"

/*
  Requirements addressed:
  - The ball spawns to the LEFT of the racket.
  - The racket is pushed back a little bit from the left (but not too far).
  - A wall behind the racket so if the ball goes past the racket, it hits the wall instead of disappearing.
  - The game runs indefinitely until 'Q' is pressed.
*/

// Ball data
static struct {
    int y, x;          // current position
    int y_dir, x_dir;  // +1 or -1 for down/up, right/left
    int y_delay, x_delay;  // "ticks" between moves for smooth motion
    int y_count, x_count;  // current counters
    char symbol;
} ball;

// Previous position to erase properly
static int old_y, old_x;

// Draw the top/bottom boundary with '-',
// left boundary with '|' at x=0,
// and a "wall behind the racket" at some column on the right.
static void draw_walls(int right_wall_col) {
    // Top & bottom
    for(int i = 0; i < COLS; i++){
        move(0, i);
        addch('-');
        move(LINES - 1, i);
        addch('-');
    }
    // Left wall
    for(int j = 0; j < LINES; j++){
        move(j, 0);
        addch('|');
    }
    // Right wall behind the paddle
    for(int j = 0; j < LINES; j++){
        move(j, right_wall_col);
        addch('|');
    }
}

// Erase the ball at (y, x). If it's on a boundary, restore that boundary's character.
static void erase_ball_at(int y, int x, int right_wall_col) {
    move(y, x);
    if(y == 0 || y == LINES - 1) {
        addch('-');
    }
    else if(x == 0) {
        addch('|');
    }
    else if(x == right_wall_col) {
        addch('|');
    }
    else {
        addch(' ');
    }
}

// Update the ball's position based on its counters
static void update_ball(int right_wall_col, int paddle_col) {
    // Erase old position
    erase_ball_at(old_y, old_x, right_wall_col);

    // Decrement counters
    if(--ball.y_count == 0) {
        ball.y += ball.y_dir;
        ball.y_count = ball.y_delay;
    }
    if(--ball.x_count == 0) {
        ball.x += ball.x_dir;
        ball.x_count = ball.x_delay;
    }

    // Draw new position
    move(ball.y, ball.x);
    addch(ball.symbol);
    refresh();

    old_y = ball.y;
    old_x = ball.x;
}

// Bounce logic
void move_ball_once(int right_wall_col, int paddle_col) {
    update_ball(right_wall_col, paddle_col);

    // Bounce off top/bottom
    if(ball.y <= 0) {
        ball.y_dir = 1;
    }
    else if(ball.y >= LINES - 1) {
        ball.y_dir = -1;
    }

    // Bounce off left wall
    if(ball.x <= 0) {
        ball.x_dir = 1;
    }
    // Bounce off right wall behind the paddle
    if(ball.x >= right_wall_col) {
        ball.x_dir = -1;
    }

    // Paddle contact => bounce left
    if(paddle_contact(ball.y, ball.x)) {
        ball.x_dir = -1;
        // Slight random tweak to horizontal speed
        int tweak = (rand() % 3) - 1;  // -1, 0, or +1
        ball.x_delay += tweak;
        if(ball.x_delay < 1) ball.x_delay = 1;
    }
}

// Initialize curses, draw walls, place the ball
void init_ball_and_walls(int right_wall_col, int paddle_col) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    srand(getpid());
    clear();

    // Draw top/bottom/left and the behind-paddle wall
    draw_walls(right_wall_col);
    refresh();

    // Initialize the ball so it spawns to the LEFT of the racket
    // We'll place the racket at paddle_col, so let's spawn ball ~10 columns left from that
    ball.y        = LINES / 2;
    ball.x        = paddle_col - 10;
    ball.y_dir    = (rand() % 2) ? 1 : -1;  // random up/down
    ball.x_dir    = 1;  // travels right
    ball.y_delay  = 2;  // adjust for speed
    ball.x_delay  = 2;
    ball.y_count  = ball.y_delay;
    ball.x_count  = ball.x_delay;
    ball.symbol   = 'o';

    old_y = ball.y;
    old_x = ball.x;
    move(ball.y, ball.x);
    addch(ball.symbol);
    refresh();
}

// End curses mode
void wrap_up() {
    endwin();
}
