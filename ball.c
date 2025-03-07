#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include "paddle.h"

/*
  Updated so that:
  - The score is displayed at the top right on row 0 and stays visible.
  - The top boundary is drawn on row 1 (leaving row 0 free for the score).
  - Points are only awarded when the ball removes a tile from the left wall.
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
static int leftWall[2000]; // For rows: 1 means tile exists, 0 means removed

// Draw boundaries starting from row 1
static void draw_walls(int right_wall_col, int paddle_col) {
    // Top boundary on row 1 and bottom boundary on row LINES-1
    for (int i = 0; i < COLS; i++){
        move(1, i);
        addch('-');
        move(LINES - 1, i);
        addch('-');
    }
    // Left wall: rows 2 to LINES-2
    for (int y = 2; y < LINES - 1; y++){
        if (leftWall[y])
            move(y, 0), addch('|');
        else
            move(y, 0), addch(' ');
    }
    // Right wall behind the paddle
    for (int j = 1; j < LINES; j++){
        move(j, right_wall_col);
        addch('|');
    }
}

// Display score at top right on row 0
static void show_score(void) {
    char buf[20];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    int pos = COLS - 15;
    if (pos < 0) pos = 0;
    move(0, pos);
    printw("%s", buf);
    refresh();
}

static void erase_ball_at(int y, int x, int right_wall_col) {
    move(y, x);
    // If the ball is on the paddle, leave the paddle intact.
    if (paddle_contact(y, x)) {
        addch('#');
        return;
    }
    // Top (boundary row is 1) or bottom
    if (y == 1 || y == LINES - 1) {
        addch('-');
        return;
    }
    // Left wall: row between 2 and LINES-2
    if (x == 0) {
        if (y >= 2 && y < LINES - 1) {
            if (leftWall[y])
                addch('|');
            else
                addch(' ');
        } else {
            addch(' ');
        }
        return;
    }
    // Right wall
    if (x == right_wall_col) {
        addch('|');
        return;
    }
    addch(' ');
}

static void update_ball(int right_wall_col) {
    erase_ball_at(old_y, old_x, right_wall_col);
    if (--ball.y_count == 0) {
        ball.y += ball.y_dir;
        ball.y_count = ball.y_delay;
    }
    if (--ball.x_count == 0) {
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

    // Bounce off top (play area starts at row 2) and bottom
    if (ball.y <= 2)
        ball.y_dir = 1;
    else if (ball.y >= LINES - 2)
        ball.y_dir = -1;

    // If ball hits the left wall at x==0, award a point ONLY if a tile exists
    if (ball.x <= 0) {
        if (ball.y >= 2 && ball.y < LINES - 1 && leftWall[ball.y] == 1) {
            leftWall[ball.y] = 0;
            score++;
            show_score();
        }
        ball.x_dir = 1;
    }
    // If ball reaches the paddle column, bounce off only if the paddle is there.
    else if (ball.x == paddle_col) {
        if (paddle_contact(ball.y, paddle_col)) {
            ball.x_dir = -1;
        }
    }
    // Bounce off the right wall behind the paddle.
    else if (ball.x >= right_wall_col) {
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
    // Initialize left wall tiles for all rows
    for (int y = 0; y < LINES; y++){
        leftWall[y] = 1;
    }
    draw_walls(right_wall_col, paddle_col);
    show_score();
    refresh();
    // Place the ball in the play area (avoid row 1)
    ball.y = LINES / 2;
    if (ball.y < 2)
        ball.y = 2;
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
    show_score();
}

void wrap_up(void) {
    endwin();
}
