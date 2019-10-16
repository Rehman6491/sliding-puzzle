/* author:     Miguel Bartelsman */

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#define TEST 0
#define DEBUG 0

typedef struct {
    int row;
    int col;
} Position;


typedef enum {
    Up = 1,
    Down = 2,
    Left = 3,
    Right = 4,
} Direction;


typedef enum {
    Solved = 1,
    Unsolved = 0,
} GameState;


typedef struct {
    int size;
    int **board;
    Position blank;
    GameState state;
    WINDOW *win;
} Game;


int boardArea(Game *game) {
    return game->size * game->size;
}


void allocateBoard(Game *game) {
    game->board = malloc(game->size * sizeof(int *));
    game->board[0] = malloc(boardArea(game) * sizeof(int));

    for (int row = 1; row < game->size; row++) {
        game->board[row] = game->board[row - 1] + game->size;
    }
}


void freeBoard(Game *game) {
    free(game->board[0]);
    free(game->board);
    game->board = NULL;
}


bool isSolvable(Game *game) {
    printf("Checking puzzle...\n");
    int inversions = 0;

    /* Count inversions -- number of occurences of lower numbers after every index */
    for (int row = 0; row < game->size; row++) {
        for (int col = 0; col < game->size; col++) {

            if (game->board[row][col] == 0) {
                game->blank.row = row;
                game->blank.col = col;
            }

            for (int n = 1; n < boardArea(game) - (row * game->size + col); n++) {
                if (game->board[row][col] > game->board[row + (n / game->size)][col + (n % game->size)]) {
                    inversions++;
                }
            }
        }
    }

    /* Sliding puzzle theory */
    if (game->size % 2) {
        if (inversions % 2 == 0) {
            printf("  Puzzle is solvable\n");
            return 1;
        } else {
            printf("  Puzzle is unsolvable, retrying\n");
            return 0;
        }
    } else {
        if (((game->size - game->blank.row) % 2) == (inversions % 2 == 0)) {
            printf("  Puzzle is solvable\n");
            return 1;
        } else {
            printf("  Puzzle is unsolvable, retrying\n");
            return 0;
        }
    }
}


void populateBoard(Game *game) {

    printf("\nBuilding puzzle...\n");
    
    Position currentPos;
    do {
        /* Initialize board */
        for (int row = 0; row < game->size; row++) {
            for (int col = 0; col < game->size; col++) {
                game->board[row][col] = -1;
            }
        }

        /* Fill the board with valid values */
        for (int n = 0; n < boardArea(game); n++) {
            
            do {
                currentPos.row = rand() % (game->size);
                currentPos.col = rand() % (game->size);
            } while (game->board[currentPos.row][currentPos.col] != -1);

            game->board[currentPos.row][currentPos.col] = n;
        }
    } while (isSolvable(game) == 0); /* Try again if puzzle is unsolvable */

    printf("Puzzle ready!\n");
}


Game init() {
    Game game;

    /* Get size */
    printf ("::: SLIDING PUZZLE :::\n\nInput the size you'd like to play (3 - 9): ");

    scanf("%d", &(game.size));

    while (game.size < 3 || 9 < game.size) {

        printf("Size must be an integer between and including 3 and 9\nTry again: ");

        scanf("%d", &(game.size));
    }

    /* Initialize game */
    allocateBoard(&game);
    populateBoard(&game);

    /* Initialize ncurses */
	initscr(); /* starts curses */
	clear(); /* Clears screen */
	cbreak(); /* Disables line buffering */
    refresh(); /* Prepares screen for output */
	noecho(); /* Disables echo */

    const int WIDTH  = game.size * 4 + 1;
    const int HEIGHT = game.size * 2 + 1;
    const int STARTX = 1;
    const int STARTY = 2;
    game.win = newwin(HEIGHT, WIDTH, STARTX, STARTY); /* Creates curses window */

    wrefresh(game.win);
	keypad(game.win, TRUE); /* Enables keypad input for window */

    return game;
}


void moveTile(Game *game, Direction dir) {
    /* Pattern matching */
    Position locus[] = {
        {   /* Up */
            .row = game->blank.row + game->size,
            .col = game->blank.col
        },{ /* Down */
            .row = game->blank.row - game->size,
            .col = game->blank.col
        },{ /* Left */
            .row = game->blank.row,
            .col = game->blank.col + 1
        },{ /* Right */
            .row = game->blank.row,
            .col = game->blank.col - 1
        }
    };

    /* Assign locations to vars for ease of access */
    int* tile = &game->board[locus[dir].row][locus[dir].col];
    int* blank = &game->board[game->blank.row][game->blank.col];

    /* Update blank location in game */
    game->blank.row = locus[dir].row;
    game->blank.col = locus[dir].col;

    /* swap values */
    *blank = *tile;
    *tile = 0;
}


void end(Game *game, int status) {
    freeBoard(game);
	clrtoeol();
    nocbreak();
	refresh();
	endwin();
    exit(status);
}


void draw(Game *game) {

    /* Print board */
    for (int x = 0; x < game->size; x++) {
        for (int y = 0; y < game->size; y++) {
            mvwprintw(game->win, y * 2, x * 4, "+---+");
            mvwprintw(game->win, y * 2 + 1, x * 4, "|   |");
        }
        mvwprintw(game->win, game->size * 2, x * 4, "+---+");
    }

    /* Print numbers */
    for (int row = 0; row < game->size; row++) {
        for (int col = 0; col < game->size; col++) {

            char tileName;
            if (game->board[row][col]) {
                tileName = (char)(game->board[row][col] + 48);
            } else {
                tileName = ' ';
            }

            mvwaddch(game->win, row * 2 + 1, col * 4 + 2, tileName);
        }
    }
    wmove(game->win, game->blank.row * 2 + 1, game->blank.col * 4 + 2);

    wrefresh(game->win);
}


void update(Game *game) {

    int input = wgetch(game->win);

    /* Parse input */
    switch (input) {
        case KEY_UP: moveTile(game, Up); break;
        case KEY_DOWN: moveTile(game, Down); break;
        case KEY_LEFT: moveTile(game, Left); break;        
        case KEY_RIGHT: moveTile(game, Right); break;
        case 'q': end(game, 0); break;
        default: break;
    }
}


int main(/*int argc, char *argv[]*/) {

    Game game = init();
    while (1) {
        draw(&game);
        update(&game);
    }
    end(&game, 0);
    return 0;
}
