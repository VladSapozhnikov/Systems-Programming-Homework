#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include "paddle.h"

// For return values
#define MISS   -1
#define NO_HIT  0
#define BOUNCE  1

struct ppball {
    int y, x;         // current position
    int y_ttm, y_ttg; // "ticks" for vertical
    int x_ttm, x_ttg; // "ticks" for horizontal
    int y_dir, x_dir; // direction: +1 or -1
    char symbol;
};

static struct ppball the_ball;
static int balls_left = 3;
static int finished   = 0;
static int score      = 0;

// We remember the ball's old position so we can fix the boundary if we erase a dash or pipe.
static int old_y, old_x;

static void draw_boundaries(void) {
    // Top and bottom dashed lines
    for(int i=0; i<COLS; i++){
        move(0, i);         addch('-');
        move(LINES-1, i);   addch('-');
    }
    // Left dashed line
    for(int j=0; j<LINES; j++){
        move(j, 0); addch('|');
    }
}

// Show score up in the corner
static void show_score(void) {
    move(0, 2);
    printw("Score: %d  Balls: %d", score, balls_left);
    refresh();
}

// Erase the old ball, but if it was on a boundary, restore the boundary character.
static void erase_ball_at(int y, int x) {
    move(y, x);
    if(y == 0 || y == LINES-1)
        addch('-');
    else if(x == 0)
        addch('|');
    else
        addch(' ');
}

static void update_ball(void) {
    // Erase old ball
    erase_ball_at(old_y, old_x);

    // Move vertically
    if(--the_ball.y_ttg == 0){
        the_ball.y    += the_ball.y_dir;
        the_ball.y_ttg = the_ball.y_ttm;
    }
    // Move horizontally
    if(--the_ball.x_ttg == 0){
        the_ball.x    += the_ball.x_dir;
        the_ball.x_ttg = the_ball.x_ttm;
    }

    // Draw new ball
    move(the_ball.y, the_ball.x);
    addch(the_ball.symbol);
    refresh();

    // Update old position
    old_y = the_ball.y;
    old_x = the_ball.x;
}

static int bounce_or_miss(void) {
    update_ball();

    // Bounce off top or bottom
    if(the_ball.y <= 0) {
        the_ball.y_dir = 1;
    } else if(the_ball.y >= LINES-1) {
        the_ball.y_dir = -1;
    }

    // Check if it went past the left edge => MISS
    if(the_ball.x <= 0) {
        return MISS;
    }
    // Or if it hits the right edge => bounce back
    if(the_ball.x >= COLS-1) {
        the_ball.x_dir = -1;
    }

    // Check paddle contact
    if(paddle_contact(the_ball.y, the_ball.x)) {
        score++;
        show_score();
        // Reverse direction
        the_ball.x_dir = 1;
        // Random tweak
        the_ball.x_ttm += (rand() % 3) - 1;
        if(the_ball.x_ttm < 1) the_ball.x_ttm = 1;
        return BOUNCE;
    }
    return NO_HIT;
}

static void serve_ball(void) {
    the_ball.y      = LINES / 2;
    the_ball.x      = COLS / 2;
    the_ball.y_ttm  = 3 + rand() % 3;
    the_ball.x_ttm  = 3 + rand() % 3;
    the_ball.y_ttg  = the_ball.y_ttm;
    the_ball.x_ttg  = the_ball.x_ttm;
    the_ball.y_dir  = (rand() % 2) ? 1 : -1;
    the_ball.x_dir  = -1;  // Ball travels left initially
    the_ball.symbol = 'o';

    // Initialize old position
    old_y = the_ball.y;
    old_x = the_ball.x;

    move(the_ball.y, the_ball.x);
    addch(the_ball.symbol);
    refresh();
}

void set_up(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    srand(getpid());
    clear();

    // Draw static boundaries once
    draw_boundaries();
    show_score();
    refresh();
}

void wrap_up(void) {
    endwin();
}

void move_ball(void) {
    int rc = bounce_or_miss();
    if(rc == MISS) {
        if(--balls_left > 0) {
            serve_ball();
            show_score();
        }
        else {
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
