#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include "paddle.h"

/*
  Points are now awarded ONLY when the ball hits a tile from the left wall.
  If the ball bounces off the paddle, no points are added.
*/

static struct {
    int y, x;
    int y_dir, x_dir;
    int y_delay, x_delay;
    int y_count, x_count;
    char symbol;
} ball;

static int old_y, old_x;
static int score = 0;
static int leftWall[2000];

static void draw_walls(int right_wall_col, int paddle_col) {
    for (int i = 0; i < COLS; i++){
        move(0, i); addch('-');
        move(LINES - 1, i); addch('-');
    }
    for (int y = 0; y < LINES; y++){
        if (y == 0 || y == LINES - 1)
            continue;
        if (leftWall[y])
            move(y, 0), addch('|');
        else
            move(y, 0), addch(' ');
    }
    for (int j = 0; j < LINES; j++){
        move(j, right_wall_col); addch('|');
    }
}

static void show_score(void) {
    move(0, 2);
    printw("Score: %d ", score);
    refresh();
}

static void erase_ball_at(int y, int x, int right_wall_col) {
    move(y, x);
    if(paddle_contact(y, x)) {
        addch('#');
        return;
    }
    if(y == 0 || y == LINES - 1) {
        addch('-');
        return;
    }
    if(x == 0) {
        if(leftWall[y])
            addch('|');
        else
            addch(' ');
        return;
    }
    if(x == right_wall_col) {
        addch('|');
        return;
    }
    addch(' ');
}

static void update_ball(int right_wall_col) {
    erase_ball_at(old_y, old_x, right_wall_col);
    if(--ball.y_count == 0) {
        ball.y += ball.y_dir;
        ball.y_count = ball.y_delay;
    }
    if(--ball.x_count == 0) {
        ball.x += ball.x_dir;
        ball.x_count = ball.x_delay;
    }
    move(ball.y, ball.x);
    addch(ball.symbol);
    refresh();
    old_y = ball.y;
    old_x = ball.x;
}

void move_ball_once(int right_wall_col, int paddle_col) {
    update_ball(right_wall_col);

    if(ball.y <= 0)
        ball.y_dir = 1;
    else if(ball.y >= LINES - 1)
        ball.y_dir = -1;

    if(ball.x <= 0) {
        if(leftWall[ball.y] == 1) {
            leftWall[ball.y] = 0;
            score++;
            show_score();
        }
        ball.x_dir = 1;
    }
    else if(ball.x == paddle_col) {
        if(paddle_contact(ball.y, paddle_col)) {
            ball.x_dir = -1;
        }
    }
    else if(ball.x >= right_wall_col) {
        ball.x_dir = -1;
    }
}

void init_ball_and_walls(int right_wall_col, int paddle_col) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    srand(getpid());
    clear();
    for(int y = 0; y < LINES; y++){
        leftWall[y] = 1;
    }
    draw_walls(right_wall_col, paddle_col);
    show_score();
    refresh();
    ball.y = LINES / 2;
    ball.x = paddle_col - 5;
    ball.y_dir = (rand() % 2) ? 1 : -1;
    ball.x_dir = 1;
    ball.y_delay = 2;
    ball.x_delay = 2;
    ball.y_count = ball.y_delay;
    ball.x_count = ball.x_delay;
    ball.symbol = 'o';
    old_y = ball.y;
    old_x = ball.x;
    move(ball.y, ball.x);
    addch(ball.symbol);
    refresh();
}

void redraw_boundaries_and_paddle(int right_wall_col, int paddle_col) {
    draw_walls(right_wall_col, paddle_col);

}

void wrap_up(void) {
    endwin();
}
