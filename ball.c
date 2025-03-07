#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include "paddle.h"

/*
    We have:
    - Walls at top (y=0) and bottom (y=LINES-1)
    - A wall at left (x=0)
    - A wall at right (x=COLS-1), purely cosmetic ("behind" the paddle)
    - The paddle near the right side (e.g. col = COLS-5)

    The ball spawns to the LEFT of the paddle, travels right.
    If the ball hits left wall (x=0), we increment score and bounce it right.
    If the ball reaches the paddle column, we check for contact:
       - If contact, bounce the ball left
       - If miss, we lose a ball
    The top/bottom walls cause vertical bounces.
    The user gets points each time the ball hits the left wall.
*/

#define MISS    -1
#define NO_HIT   0
#define BOUNCE   1

struct ppball {
    int y, x;          // Current position
    int y_ttm, y_ttg;  // Vertical "ticks" total and "ticks" left
    int x_ttm, x_ttg;  // Horizontal "ticks" total and "ticks" left
    int y_dir, x_dir;  // +1 (down/right) or -1 (up/left)
    char symbol;
};

static struct ppball the_ball;
static int balls_left = 3;
static int finished   = 0;
static int score      = 0;

// Old position of the ball to erase it properly
static int old_y, old_x;

// Draw top/bottom walls and left/right walls
static void draw_walls(void) {
    // Top and bottom horizontal lines
    for(int i = 0; i < COLS; i++) {
        move(0, i);            addch('-');
        move(LINES - 1, i);    addch('-');
    }
    // Left wall
    for(int j = 0; j < LINES; j++) {
        move(j, 0); addch('|');
    }
    // Wall behind the paddle at the far right
    for(int j = 0; j < LINES; j++) {
        move(j, COLS - 1); addch('|');
    }
}

// Display score and remaining balls at top
static void show_status(void) {
    move(0, 2);
    printw("Score: %d   Balls: %d", score, balls_left);
    refresh();
}

// Erase the ball at old position. If it's on a wall, restore the wall char
static void erase_ball_at(int y, int x) {
    move(y, x);
    if(y == 0 || y == LINES - 1) {
        addch('-');
    } 
    else if(x == 0) {
        addch('|');
    }
    else if(x == COLS - 1) {
        addch('|');
    }
    else {
        addch(' ');
    }
}

// Move the ball according to its "ticks"
static void update_ball(void) {
    // Erase old
    erase_ball_at(old_y, old_x);

    // Vertical motion
    if(--the_ball.y_ttg == 0) {
        the_ball.y += the_ball.y_dir;
        the_ball.y_ttg = the_ball.y_ttm;
    }
    // Horizontal motion
    if(--the_ball.x_ttg == 0) {
        the_ball.x += the_ball.x_dir;
        the_ball.x_ttg = the_ball.x_ttm;
    }

    // Draw new
    move(the_ball.y, the_ball.x);
    addch(the_ball.symbol);
    refresh();

    // Update old coords
    old_y = the_ball.y;
    old_x = the_ball.x;
}

// Check for bounces or misses
static int bounce_or_miss(void) {
    update_ball();

    // Bounce off top
    if(the_ball.y <= 0) {
        the_ball.y_dir = 1;
    }
    // Bounce off bottom
    else if(the_ball.y >= LINES - 1) {
        the_ball.y_dir = -1;
    }

    // If ball hits left wall => score a point, bounce right
    if(the_ball.x <= 0) {
        score++;
        show_status();
        the_ball.x_dir = 1;
    }

    // If ball goes beyond or at the paddle column => check paddle
    // Paddle is near the right side. If the ball passes the paddle column 
    // and no contact => MISS (lose ball).
    if(the_ball.x >= (COLS - 5)) {
        if(paddle_contact(the_ball.y, the_ball.x)) {
            // Bounce left
            the_ball.x_dir = -1;
            // Random tweak to horizontal speed
            the_ball.x_ttm += (rand() % 3) - 1;
            if(the_ball.x_ttm < 1) the_ball.x_ttm = 1;
            return BOUNCE;
        } 
        else {
            return MISS;
        }
    }
    return NO_HIT;
}

// Serve a new ball
static void serve_ball(void) {
    // Spawn the ball LEFT of the racket
    the_ball.y      = LINES / 2;
    the_ball.x      = (COLS - 5) - 5;  // e.g. racket at col=COLS-5, ball ~ 5 columns left
    the_ball.y_ttm  = 3 + rand() % 3;
    the_ball.x_ttm  = 3 + rand() % 3;
    the_ball.y_ttg  = the_ball.y_ttm;
    the_ball.x_ttg  = the_ball.x_ttm;
    // Ball travels right
    the_ball.y_dir  = (rand() % 2) ? 1 : -1; 
    the_ball.x_dir  = 1;
    the_ball.symbol = 'o';

    old_y = the_ball.y;
    old_x = the_ball.x;
    move(the_ball.y, the_ball.x);
    addch(the_ball.symbol);
    refresh();
}

// Set up curses and draw boundaries
void set_up(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    srand(getpid());
    clear();
    draw_walls();
    show_status();
    refresh();
}

void wrap_up(void) {
    endwin();
}

// Called in main loop to move the ball each tick
void move_ball(void) {
    int rc = bounce_or_miss();
    if(rc == MISS) {
        if(--balls_left > 0) {
            serve_ball();
            show_status();
        } else {
            finished = 1;
        }
    }
}

int is_finished(void) {
    return finished;
}

int get_balls_left(void) {
    return balls_left;
}

void start_game(void) {
    serve_ball();
}
