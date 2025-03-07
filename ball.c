#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include "paddle.h"

#define MISS     -1
#define NO_HIT    0
#define BOUNCE    1

struct ppball {
    int y, x;
    int y_ttm, y_ttg;
    int x_ttm, x_ttg;
    int y_dir, x_dir;
    char symbol;
};

static struct ppball the_ball;
static int balls_left = 3;
static int finished = 0;

static void update_ball() {
    move(the_ball.y, the_ball.x);
    addch(' ');
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
}

static int bounce_or_miss() {
    update_ball();
    if(the_ball.y <= 0) the_ball.y_dir = 1;
    else if(the_ball.y >= LINES-1) the_ball.y_dir = -1;
    if(the_ball.x <= 0) the_ball.x_dir = 1;
    else if(the_ball.x >= COLS-1) return MISS;
    if(paddle_contact(the_ball.y, the_ball.x)) {
        the_ball.x_dir = -1;
        the_ball.x_ttm += (rand() % 3) - 1;
        if(the_ball.x_ttm < 1) the_ball.x_ttm = 1;
        return BOUNCE;
    }
    return NO_HIT;
}

void set_up() {
    initscr();
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    bkgd(COLOR_PAIR(1));
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    srand(getpid());
    clear();
    for(int i = 0; i < COLS-1; i++){
        move(0, i); addch('-');
        move(LINES-1, i); addch('-');
    }
    for(int j = 0; j < LINES; j++){
        move(j, 0); addch('|');
    }
    refresh();
}

void wrap_up() {
    endwin();
}

static void serve_ball() {
    the_ball.y = LINES/2;
    the_ball.x = COLS/2;
    the_ball.y_ttm = 3 + rand()%3;
    the_ball.x_ttm = 3 + rand()%2;
    the_ball.y_ttg = the_ball.y_ttm;
    the_ball.x_ttg = the_ball.x_ttm;
    the_ball.y_dir = (rand() % 2) ? 1 : -1;
    the_ball.x_dir = -1;
    the_ball.symbol = 'o';
    move(the_ball.y, the_ball.x);
    addch(the_ball.symbol);
    refresh();
}

void move_ball() {
    int rc = bounce_or_miss();
    if(rc == MISS) {
        if(--balls_left > 0) serve_ball();
        else finished = 1;
    }
}

int is_finished() {
    return finished;
}

int get_balls_left() {
    return balls_left;
}

void start_game() {
    serve_ball();
}
