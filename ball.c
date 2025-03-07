#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include "paddle.h"

/*
  Minor changes from the previous version:
   1) The paddle (#) does NOT vanish when the ball passes over it.
   2) The left wall is made of tiles that disappear as the ball hits them,
      adding 1 point for each tile removed.
   3) The game runs until 'Q' is pressed.
*/

// Ball data
static struct {
    int y, x;           // current position
    int y_dir, x_dir;   // +1 or -1 for down/up, right/left
    int y_delay, x_delay;  // "ticks" between moves
    int y_count, x_count;  // counters
    char symbol;
} ball;

// Old position for erasing
static int old_y, old_x;

// Score
static int score = 0;

// We'll store whether each row of the left wall is still present.
static int leftWall[2000]; 
// leftWall[y] = 1 means tile is there, 0 means tile was removed by a ball hit.

// We draw the top & bottom lines and the wall behind the paddle.
// The left boundary is drawn tile by tile based on leftWall[y].
static void draw_walls(int right_wall_col, int paddle_col) {
    // Top & bottom horizontal lines
    for(int i = 0; i < COLS; i++){
        move(0, i);           addch('-');
        move(LINES - 1, i);   addch('-');
    }
    // Left wall "tiles"
    for(int y = 0; y < LINES; y++){
        if(y == 0 || y == LINES - 1) {
            // Already drawn above as top/bottom lines
            continue;
        }
        if(leftWall[y]) {
            move(y, 0);
            addch('|');
        } else {
            move(y, 0);
            addch(' ');
        }
    }
    // Wall behind the paddle (on the right)
    for(int j = 0; j < LINES; j++){
        move(j, right_wall_col);
        addch('|');
    }
    // Re-draw paddle in case it was overwritten
    // We'll do that in main after we handle input.
}

// A quick scoreboard
static void show_score(void) {
    move(0, 2);
    printw("Score: %d ", score);
    refresh();
}

// Erase the ball at (y, x). If that's on a boundary or a paddle, restore it properly
static void erase_ball_at(int y, int x, int right_wall_col) {
    move(y, x);

    // If it's on the paddle, do NOT erase the paddle (#).
    if(paddle_contact(y, x)) {
        addch('#');
        return;
    }
    // If top/bottom lines
    if(y == 0 || y == LINES - 1) {
        addch('-');
        return;
    }
    // If it's the right wall behind the paddle
    if(x == right_wall_col) {
        addch('|');
        return;
    }
    // If it's the left boundary tile (and still present)
    if(x == 0) {
        if(leftWall[y]) {
            addch('|');
            return;
        } else {
            addch(' ');
            return;
        }
    }
    // Otherwise just erase
    addch(' ');
}

// Update the ball's position with its counters
static void update_ball(int right_wall_col) {
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

    // Draw new
    move(ball.y, ball.x);
    addch(ball.symbol);
    refresh();

    old_y = ball.y;
    old_x = ball.x;
}

// Move ball once, handle hits
void move_ball_once(int right_wall_col, int paddle_col) {
    update_ball(right_wall_col);

    // Bounce off top/bottom
    if(ball.y <= 0) {
        ball.y_dir = 1;
    } 
    else if(ball.y >= LINES - 1) {
        ball.y_dir = -1;
    }

    // If we hit the left boundary (x==0) and there's still a tile
    if(ball.x == 0 && leftWall[ball.y] == 1) {
        // Remove that tile, add 1 point
        leftWall[ball.y] = 0;
        score++;
        show_score();
        // Typically, we'd bounce or keep traveling. Let's bounce it right
        ball.x_dir = 1;
    }

    // If we hit the wall behind the paddle
    if(ball.x >= right_wall_col) {
        ball.x_dir = -1;
    }

    // If we contact the paddle => bounce left, speed tweak
    if(paddle_contact(ball.y, ball.x)) {
        ball.x_dir = -1;
        int tweak = (rand() % 3) - 1;  // -1, 0, or +1
        ball.x_delay += tweak;
        if(ball.x_delay < 1) ball.x_delay = 1;
    }
}

// Initialize curses, walls, and the ball
void init_ball_and_walls(int right_wall_col, int paddle_col) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    srand(getpid());
    clear();

    // Initialize leftWall tiles
    for(int y = 0; y < LINES; y++){
        leftWall[y] = 1; // all present except top/bottom, but let's just keep it for consistency
    }

    // Draw static boundaries
    draw_walls(right_wall_col, paddle_col);
    show_score();
    refresh();

    // Place the ball to the LEFT of the paddle
    ball.y        = LINES / 2;
    ball.x        = paddle_col - 5;  // 5 columns to the left of the paddle
    ball.y_dir    = (rand() % 2) ? 1 : -1;  // random up/down
    ball.x_dir    = 1;   // heading right
    ball.y_delay  = 2;
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

void redraw_boundaries_and_paddle(int right_wall_col, int paddle_col) {
    draw_walls(right_wall_col, paddle_col);
    show_score();
}
