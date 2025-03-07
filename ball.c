#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include "paddle.h"

#define MISS   -1
#define NO_HIT  0
#define BOUNCE  1

struct ppball {
    int y, x;          // Current position
    int y_ttm, y_ttg;  // "ticks" for vertical motion
    int x_ttm, x_ttg;  // "ticks" for horizontal motion
    int y_dir, x_dir;  // +1 or -1
    char symbol;
};

static struct ppball the_ball;
static int balls_left = 3;
static int finished   = 0;
static int score      = 0;

// Track the old position of the ball so we can restore boundaries.
static int old_y, old_x;

// Re-draw the top, bottom, and left boundaries as dashed lines/pipes.
static void draw_boundaries(void) {
    // Top & bottom
    for(int i = 0; i < COLS; i++) {
        move(0, i);           addch('-');
        move(LINES - 1, i);   addch('-');
    }
    // Left side
    for(int j = 0; j < LINES; j++){
        move(j, 0); addch('|');
    }
}

// Show score and balls
static void show_status(void) {
    move(0, 2);
    printw("Score: %d   Balls: %d", score, balls_left);
    refresh();
}

// If the ball was on a boundary, restore it; otherwise just erase with a space.
static void erase_ball_at(int y, int x) {
    move(y, x);
    if(y == 0 || y == LINES - 1)
        addch('-');
    else if(x == 0)
        addch('|');
    else
        addch(' ');
}

static void update_ball(void) {
    // Erase old ball
    erase_ball_at(old_y, old_x);

    // Vertical motion
    if(--the_ball.y_ttg == 0){
        the_ball.y += the_ball.y_dir;
        the_ball.y_ttg = the_ball.y_ttm;
    }

    // Horizontal motion
    if(--the_ball.x_ttg == 0){
        the_ball.x += the_ball.x_dir;
        the_ball.x_ttg = the_ball.x_ttm;
    }

    // Draw new ball
    move(the_ball.y, the_ball.x);
    addch(the_ball.symbol);
    refresh();

    old_y = the_ball.y;
    old_x = the_ball.x;
}

static int bounce_or_miss(void) {
    update_ball();

    // Bounce off top/bottom
    if(the_ball.y <= 0){
        the_ball.y_dir = 1;
    } 
    else if(the_ball.y >= LINES - 1){
        the_ball.y_dir = -1;
    }

    // If ball reaches left edge => MISS
    if(the_ball.x <= 0){
        return MISS;
    }
    // If ball hits right edge => bounce left
    else if(the_ball.x >= COLS - 1){
        the_ball.x_dir = -1;
    }

    // Paddle contact => bounce the ball left
    if(paddle_contact(the_ball.y, the_ball.x)) {
        the_ball.x_dir = -1;
        score++;
        show_status();

        // Random tweak to speed
        the_ball.x_ttm += (rand() % 3) - 1;
        if(the_ball.x_ttm < 1) the_ball.x_ttm = 1;
        return BOUNCE;
    }

    return NO_HIT;
}

static void serve_ball(void) {
    the_ball.y      = LINES / 2;
    the_ball.x      = COLS / 4;     // Start from roughly the left quarter
    the_ball.y_ttm  = 3 + rand()%3;
    the_ball.x_ttm  = 3 + rand()%3;
    the_ball.y_ttg  = the_ball.y_ttm;
    the_ball.x_ttg  = the_ball.x_ttm;
    the_ball.y_dir  = (rand() % 2) ? 1 : -1;
   
