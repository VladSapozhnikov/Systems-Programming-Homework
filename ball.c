#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include "paddle.h"

#define MISS   -1
#define NO_HIT  0
#define BOUNCE  1

struct ppball {
    int y, x;
    int y_ttm, y_ttg;
    int x_ttm, x_ttg;
    int y_dir, x_dir;
    char symbol;
};

static struct ppball the_ball;
static int balls_left = 3;
static int finished   = 0;
static int score      = 0;
static int old_y, old_x;

static void draw_walls(void) {
    for(int i = 0; i < COLS; i++) {
        move(0, i);           addch('-');
        move(LINES - 1, i);   addch('-');
    }
    for(int j = 0; j < LINES; j++) {
        move(j, 0); addch('|');
    }
}

static void show_status(void) {
    move(0, 2);
    printw("Score: %d   Balls: %d", score, balls_left);
    refresh();
}

static void erase_ball_at(int y, int x) {
    move(y, x);
    if(y == 0 || y == LINES - 1) addch('-');
    else if(x == 0) addch('|');
    else addch(' ');
}

static void update_ball(void) {
    erase_ball_at(old_y, old_x);
    if(--the_ball.y_ttg == 0) {
        the_ball.y += the_ball.y_dir;
        the_ball.y_ttg = the_ball.y_ttm;
    }
    if(--the_ball.x_ttg == 0) {
        the_ball.x += the_ball.x_dir;
        the_ball.x_ttg = the_ball.x_ttm;
    }
    move(the_ball.y, the_ball.x);
    addch(the_ball.symbol);
    refresh();
    old_y = the_ball.y;
    old_x = the_ball.x;
}

static int bounce_or_miss(void) {
    update_ball();
    if(the_ball.y <= 0)  the_ball.y_dir = 1;
    if(the_ball.y >= LINES - 1) the_ball.y_dir = -1;
    if(the_ball.x <= 0)  the_ball.x_dir = 1;
    if(the_ball.x >= COLS - 1) {
        if(!paddle_contact(the_ball.y, the_ball.x)) return MISS;
    }
    if(paddle_contact(the_ball.y, the_ball.x)) {
        score++;
        show_status();
        the_ball.x_dir = -1;
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
    the_ball.x_dir  = 1;
    the_ball.symbol = 'o';
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
    draw_walls();
    show_status();
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
