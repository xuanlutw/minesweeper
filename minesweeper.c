# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>
# include <termios.h>
# include <unistd.h>

typedef struct {
    int width;
    int mines;
    int* loc;
    int* show;
    int* flag;
} Board;

# define Loc(B, x, y)  ((B)->loc[(x) * (B)->width + (y)])
# define Show(B, x, y) ((B)->show[(x) * (B)->width + (y)])
# define Flag(B, x, y) ((B)->flag[(x) * (B)->width + (y)])
# define In(B, x, y)   ((x) >= 0 && x < (B)->width && (y) >= 0 && (y) < (B)->width)
# define MINE -1

# define Surround(x, y, G) \
    G((x) + 1, (y) + 1) \
    G((x) + 1, (y) - 1) \
    G((x) + 1, (y)) \
    G((x) - 1, (y) + 1) \
    G((x) - 1, (y) - 1) \
    G((x) - 1, (y)) \
    G((x), (y) + 1) \
    G((x), (y) - 1)

# define Update_mine(a, b) \
    Loc(board, x, y) += (In(board, a, b) && (Loc(board, a, b) == MINE));
Board* init_board (int width, int mines) {
    if (mines > width * width) {
        fprintf(stderr, "Too many mines!!!\n");
        abort();
    }
    Board* board = malloc(sizeof(Board));
    board->width   = width;
    board->mines = mines;
    board->loc   = malloc(sizeof(int) * width * width);
    board->show  = malloc(sizeof(int) * width * width);
    board->flag  = malloc(sizeof(int) * width * width);
    memset(board->loc,  0, sizeof(int) * width * width);
    memset(board->show, 0, sizeof(int) * width * width);
    memset(board->flag, 0, sizeof(int) * width * width);

    srand(time(NULL));
    for (int i = 0; i < mines; ++i) {
        int x, y;
        do {
            x = rand() % width;
            y = rand() % width;
        } while (Loc(board, x, y) == MINE);
        Loc(board, x, y) = MINE;
    }
    
    for (int x = 0; x < width; ++x)
        for (int y = 0; y < width; ++y)
            if (Loc(board, x, y) == MINE)
                continue;
            else {
                Surround(x, y, Update_mine)
            }

    return board;
}

#define cusor_set(x, y)     printf("\033[%d;%dH", (x), (y));fflush(stdout)
#define erase_display_all() printf("\033[2J");fflush(stdout)
void print_board (Board* board, int cx, int cy) {
    erase_display_all();
    cusor_set(0, 0);
    for (int x = 0; x < board->width; ++x) {
        printf("%2d ", x + 1);
        for (int y = 0; y < board->width; ++y)
            //if (Loc(board, x, y) != MINE || Show(board, x, y)) {
            if (Show(board, x, y)) {
                printf("\033[40m");
                if (Loc(board, x, y) == MINE)
                    printf("\033[91mX");
                else if (Loc(board, x, y) == 0)
                    printf(" ");
                else if (Loc(board, x, y) == 1)
                    printf("\033[34m1");
                else if (Loc(board, x, y) == 2)
                    printf("\033[32m2");
                else if (Loc(board, x, y) == 3)
                    printf("\033[31m3");
                else if (Loc(board, x, y) == 4)
                    printf("\033[36m4");
                else if (Loc(board, x, y) == 5)
                    printf("\033[94m5");
                else if (Loc(board, x, y) == 6)
                    printf("\033[92m6");
                else if (Loc(board, x, y) == 7)
                    printf("\033[91m7");
                else if (Loc(board, x, y) == 8)
                    printf("\033[96m8");
            }
            else
                if Flag(board, x, y)
                    printf("\033[96m\033[100mO");
                else
                    printf("\033[100m ");
        printf("\033[37m\033[40m\n");
    }
    printf("   ");
    for (int y = 0; y < board->width; ++y)
        printf("%d", (y + 1) % 10);
    printf("\n");
    cusor_set(cx + 1, cy + 4);
    return;
}

# define Flood(A, B) \
    if (In(board, (A), (B)) && !Show(board, (A), (B))) { \
        fl = 1;                                          \
        Show(board, (A), (B)) = 1;                       \
    }
void max_board(Board* board) {
    int fl = 0;
    while (1) {
        fl = 0;
        for (int x = 0; x < board->width; ++x)
            for (int y = 0; y < board->width; ++y)
                if (!Show(board, x, y) || Loc(board, x, y))
                    continue;
                else {
                    Surround(x, y, Flood)
                }
        if (!fl)
            return;
    }
}

# define WIN  1
# define LOSE 2
int check_win (Board* board) {
    int count = 0;
    for (int x = 0; x < board->width; ++x)
        for (int y = 0; y < board->width; ++y) {
            if (Show(board, x, y) && Loc(board, x, y) == MINE)
                return LOSE;
            count += 1 - Show(board, x, y);
        }
    if (count == board->mines)
        return WIN;
    else
        return 0;
}

int update_board (Board* board, int x, int y) {
    if (Show(board, x, y) == 1)
        return 0;
    else if (Loc(board, x, y) == MINE) {
        Show(board, x, y) = 1;
        return LOSE;
    }
    else {
        Show(board, x, y) = 1;
        max_board(board);
        return check_win(board);
    }
}

# define Count(x, y) \
    count += In(board, x, y) && !Show(board, x, y) && Flag(board, x, y);
# define Show_all(x, y) \
    if (In(board, x, y)) \
        Show(board, x, y) = Show(board, x, y) || (1 - Flag(board, x, y));
int label_board (Board* board, int x, int y) {
    if (Show(board, x, y)) {
        int count = 0;
        Surround(x, y, Count)
        if (count == Loc(board, x, y)) {
            Surround(x, y, Show_all);
            max_board(board);
        }
    }
    else
        Flag(board, x, y) = 1 - Flag(board, x, y);
    return check_win(board);
}

int main () {
    int width, mines, x = 0, y = 0, status = 0;
    char op;
    printf("Welcome to minesweeper!\n");
    printf("Width = ");
    scanf("%d", &width);
    printf("#Mines = ");
    scanf("%d", &mines);
    getchar();

    static struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    Board* B = init_board(width, mines);
    print_board(B, 0, 0);

    while (1) {
        op = getchar();
        if (op == 'w' && In(B, x - 1, y))
            --x;
        if (op == 's' && In(B, x + 1, y))
            ++x;
        if (op == 'a' && In(B, x, y - 1))
            --y;
        if (op == 'd' && In(B, x, y + 1))
            ++y;
        if (op == 10)
            status = update_board(B, x, y);
        if (op == ' ')
            status = label_board(B, x, y);
        
        print_board(B, x, y);
        if (status == WIN) {
            cusor_set(width + 3, 0);
            printf("You win!\n");
            break;
        }
        else if (status == LOSE) {
            cusor_set(width + 3, 0);
            printf("You lose!\n");
            break;
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return 0;
}
