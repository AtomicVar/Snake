/*
 * Snake
 * 
 * Author: Guo Shuai
 * Date: 2018.02.06
 * 
 * TODO: 按原方向走会加速的问题
 *       可以原路返回的问题
 */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

enum DIREC
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NUL
};

typedef struct body
{
    int x;
    int y;
} * PtrToBody;

typedef struct body *Apple;

typedef struct SnakeRcrd
{
    int head;
    int tail;
    struct body bodies[500];
    int len;
    enum DIREC direction;
} * Snake;

void pwelcome();
void game();
Snake createSnake(WINDOW *win, int len);
void drawSnake(WINDOW *win, Snake S);
void updateSnake(WINDOW *win, Snake S, enum DIREC d, bool grow, Apple A);
bool notDied(WINDOW *win, Snake S);
Apple newApple(WINDOW *win);
bool eatApple(WINDOW *win, Snake S, Apple A, enum DIREC d);
int toEven(int n);

int main()
{
    initscr();
    clear();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();

    pwelcome(); // Print welcome interface

    char opt = getch(); // Press any key to start, q to quit
    if (opt == 'q')
    {
        endwin();
        return 0;
    }
    clear();
    game();

    getch();
    endwin();
    return 0;
}

void pwelcome()
{
    char *gameName = "Snake";
    char *authorMsg = "Author: Guo Shuai";
    char *versionInfo = "Version: 2018.2.26";
    char *pats = "Press any key to start...";

    int height = 5, width = 25;
    WINDOW *title = newwin(height, width, (LINES - height) / 2, (COLS - width) / 2);
    box(title, 0, 0);
    refresh();
    wrefresh(title);

    attron(A_BOLD | A_UNDERLINE);
    mvprintw(LINES / 2 - 2, (COLS - strlen(gameName)) / 2, "%s", gameName);
    attroff(A_BOLD | A_UNDERLINE);

    mvprintw(LINES / 2 - 1, (COLS - strlen(authorMsg)) / 2, "%s", authorMsg);
    mvprintw(LINES / 2, (COLS - strlen(versionInfo)) / 2, "%s", versionInfo);
    mvprintw(LINES / 2 + 3, (COLS - strlen(pats)) / 2, "%s", pats);
}

void game()
{
    int areaH = 20, areaW = 40;
    WINDOW *area = newwin(areaH, areaW, (LINES - areaH) / 2, (COLS - areaW) / 2 - 15);
    box(area, 0, 0);
    refresh();
    wrefresh(area);

    Snake S = createSnake(area, 3);

    drawSnake(area, S);
    Apple A = newApple(area);

    enum DIREC d = RIGHT;
    while (notDied(area, S))
    {
        int status = halfdelay(10);
        int ch = getch();
        if (status != ERR)
        {
            switch (ch)
            {
            case KEY_UP:
                d = UP;
                break;
            case KEY_LEFT:
                d = LEFT;
                break;
            case KEY_DOWN:
                d = DOWN;
                break;
            case KEY_RIGHT:
                d = RIGHT;
                break;
            }
        }
        if (eatApple(area, S, A, d)){
            updateSnake(area, S, d, true, A);
            A = newApple(area);
        }
        else
            updateSnake(area, S, d, false, A);
    }

    refresh();
    wrefresh(area);
}

Snake createSnake(WINDOW *win, int len)
{
    int row, col;
    getmaxyx(win, row, col);

    Snake S = (Snake)malloc(sizeof(struct SnakeRcrd));
    S->len = len;
    S->head = -1;
    S->tail = 0;

    int offset = 0;
    while (offset < len * 2)
    {
        PtrToBody ptr = S->bodies + (++S->head);
        ptr->x = (col - len) / 2 + offset;
        ptr->y = row / 2;
        offset += 2;
    }
    return S;
}

void drawSnake(WINDOW *win, Snake S)
{
    init_pair(1, COLOR_GREEN, COLOR_YELLOW);

    wattron(win, COLOR_PAIR(1) | A_BOLD);
    for (int i = S->tail; i <= S->head; i++)
    {
        PtrToBody ptr = S->bodies + i;
        mvwprintw(win, ptr->y, ptr->x, "##");
    }
    wattroff(win, COLOR_PAIR(1) | A_BOLD);

    refresh();
    wrefresh(win);
}

void updateSnake(WINDOW *win, Snake S, enum DIREC direction, bool grow, Apple A)
{
    init_pair(1, COLOR_GREEN, COLOR_YELLOW);
    wattron(win, COLOR_PAIR(1) | A_BOLD);

    if (grow) {
        S->len++;
        struct body new;
        new.x = A->x;
        new.y = A->y;
        S->bodies[++S->head] = new;

        /* Change the apple to the snake's body */
        mvwprintw(win, S->bodies[S->head].y, S->bodies[S->head].x, "##");
    }

    struct body new;
    switch (direction)
    {
    case UP:
        new.x = S->bodies[S->head].x;
        new.y = S->bodies[S->head].y - 1;
        break;
    case DOWN:
        new.x = S->bodies[S->head].x;
        new.y = S->bodies[S->head].y + 1;
        break;
    case LEFT:
        new.x = S->bodies[S->head].x - 2;
        new.y = S->bodies[S->head].y;
        break;
    case RIGHT:
        new.x = S->bodies[S->head].x + 2;
        new.y = S->bodies[S->head].y;
        break;
    }
    S->bodies[++S->head] = new;

    /* Add a new head */
    mvwprintw(win, S->bodies[S->head].y, S->bodies[S->head].x, "##");

    wattroff(win, COLOR_PAIR(1) | A_BOLD);
    /* Erase a tail */
    mvwprintw(win, S->bodies[S->tail].y, S->bodies[S->tail].x, "  ");
    S->tail++;

    refresh();
    wrefresh(win);
}

bool notDied(WINDOW *win, Snake S)
{
    /*
     * TODO: 吃自己也应当死亡
     */
    int row, col;
    getmaxyx(win, row, col);
    int cx = S->bodies[S->head].x;
    int cy = S->bodies[S->head].y;
    if (cx > col || cx < 0 || cy < 0 || cy > row)
        return false; // Died
    else
        return true;
}

Apple newApple(WINDOW *win)
{
    /*
     * TODO: 解决产生苹果会到边界的问题
     */
    int row, col;
    Apple A = (Apple)malloc(sizeof(struct body));

    getmaxyx(win, row, col);
    srand(time(NULL));
    A->x = 2 + toEven(rand() % (col - 4));
    A->y = 2 + toEven(rand() % (row - 4));

    init_pair(2, COLOR_GREEN, COLOR_RED);

    wattron(win, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(win, A->y, A->x, "@@");
    wattroff(win, COLOR_PAIR(2) | A_BOLD);

    refresh();
    wrefresh(win);
    return A;
}

bool eatApple(WINDOW *win, Snake S, Apple A, enum DIREC d)
{
    int nextX, nextY;
    switch (d) {
        case UP:
            nextX = S->bodies[S->head].x;
            nextY = S->bodies[S->head].y - 1;
            break;
        case DOWN:
            nextX = S->bodies[S->head].x;
            nextY = S->bodies[S->head].y + 1;
            break;
        case LEFT:
            nextX = S->bodies[S->head].x - 2;
            nextY = S->bodies[S->head].y;
            break;
        case RIGHT:
            nextX = S->bodies[S->head].x + 2;
            nextY = S->bodies[S->head].y;
            break;
    }
    if (nextX == A->x && nextY == A->y)
        return true;
    else
        return false;
}

int toEven(int n){
    if (n % 2 != 0)
        n++;
    return n;
}