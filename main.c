#include<stdio.h>

#define WIDTH 8      // width of board
#define HEIGHT 8     // height (same as width)
#define BLACK '*'    // what represents black's piece
#define WHITE 'o'    // white
#define PLACABLE '.' // placable cells
#define NONE ' '     // for spaces

// mask for searching
#define LMASK 0xfefefefefefefefeul // Left
#define RMASK 0x7f7f7f7f7f7f7f7ful // Right
#define HMASK LMASK & RMASK        // Horizontal
#define TMASK 0xffffffffffffff00ul // Top
#define BMASK 0x00fffffffffffffful // Bottom
#define VMASK TMASK & BMASK        // Vertical

/*
 * @brief Counts standing bit
 * @param num: 数える対象
 * @return number of standing bit
*/
int countb(unsigned long num) {
    num = (num >> 1  & 0x5555555555555555ul) + (num & 0x5555555555555555ul);
    num = (num >> 2  & 0x3333333333333333ul) + (num & 0x3333333333333333ul);
    num = (num >> 4  & 0x0f0f0f0f0f0f0f0ful) + (num & 0x0f0f0f0f0f0f0f0ful);
    num = (num >> 8  & 0x00ff00ff00ff00fful) + (num & 0x00ff00ff00ff00fful);
    num = (num >> 16 & 0x0000ffff0000fffful) + (num & 0x0000ffff0000fffful);
    return (num >> 32) + (num & 0x00000000fffffffful);
}

/*
 * @brief Prints bitboard
 * @param num: print対象
*/
void printb(unsigned long num) {
    // あの上のあれ
    putchar(' ');
    for (int i = 0; i < WIDTH; i++) printf(" %c", 'A' + i);

    // loop for 64 bits
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        if (!(i % WIDTH)) printf("\n%d ", i / WIDTH + 1);

        if (num >> i & 1) putchar(PLACABLE);
        else              putchar(NONE);
        putchar(NONE);
    }
    putchar('\n');
}

/*
 * @brief View game status
 * @params black:    black's bitboard
 *         white:    white's bitboard
 *         placable: bitboard representing placable cells
*/
void view(
    unsigned long black,
    unsigned long white,
    unsigned long placable
) {
    putchar(' ');
    for (int i = 0; i < WIDTH; i++) printf(" %c", 'A' + i);

    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        if (!(i % WIDTH)) printf("\n%d ", i / WIDTH + 1);

        if (black >> i & 1)         putchar(BLACK);
        else if (white >> i & 1)    putchar(WHITE);
        else if (placable >> i & 1) putchar(PLACABLE);
        else                        putchar(NONE);
        putchar(NONE);
    }
    puts("\n");

    printf("BLACK %d : %d WHITE\n", countb(black), countb(white));
}

/*
 * @brief Change coordinate to bitboard
 * @params x: x coordinate (A - H)
 *         y: y coordinate (1 - 8)
 * @return bitboard
*/
unsigned long c2b(
    int x,
    int y
) {
    if (
        x < 0 ||
        x >= WIDTH ||
        y < 0 ||
        y >= HEIGHT
    )    return 0;
    else return 1lu << (y * WIDTH + x);
}

/*
 * @brief Get placable cells
 * @params p: player's bitboard
 *         o: opponent's bitboard
 * @return placable cells in bitboard
*/
unsigned long get_placable(
    unsigned long p,
    unsigned long o
) {
    unsigned long
        hm = o & HMASK,
        vm = o & VMASK,
        dw = hm & vm,

        hb  = hm & (p >> 1 | p << 1),
        vb  = vm & (p >> WIDTH | p << WIDTH),
        db1 = dw & (p >> (WIDTH + 1) | p << (WIDTH + 1)),
        db2 = dw & (p >> (WIDTH - 1) | p << (WIDTH - 1));
    
    for (int i = 0; i < 5; i++) {
        hb  |= hm & (hb >> 1 | hb << 1);
        vb  |= vm & (vb >> WIDTH | vb << WIDTH);
        db1 |= dw & (db1 >> (WIDTH + 1) | db1 << (WIDTH + 1));
        db2 |= dw & (db2 >> (WIDTH - 1) | db2 << (WIDTH - 1));
    }

    hb  = hb >> 1 | hb << 1;
    vb  = vb >> WIDTH | vb << WIDTH;
    db1 = db1 >> (WIDTH + 1) | db1 << (WIDTH + 1);
    db2 = db2 >> (WIDTH - 1) | db2 << (WIDTH - 1);

    return ~(p | o) & (hb | vb | db1 | db2);
}

/*
 * @brief Transfer cell for searching
 * @params hand: input cell
 *         mask: mask to prevent searching outside of board
 *         dir:  searching direction
 * @return transfered bitboard
*/
unsigned long transfer(
    unsigned long hand,
    unsigned long mask,
    int dir
) {
    if (dir > 0) hand >>= dir;
    else         hand <<= -dir;
    return mask & hand;
}

/*
 * @brief Flipping pieces
 * @params hand: input cell
 *         *p:   pointer of player bitboard
 *         *o:   pointer of opponent bitboard
 * @return flipped place in bitboard
*/
unsigned long flip(
    unsigned long hand,
    unsigned long *p,
    unsigned long *o
) {
    int dir[] = {
        -1,
        1,
        1 - WIDTH,
        WIDTH - 1,
        -WIDTH,
        WIDTH,
        -1 - WIDTH,
        WIDTH + 1
    };
    unsigned long
        mask[] = {
            LMASK,
            RMASK,
            RMASK & TMASK,
            LMASK & BMASK,
            TMASK,
            BMASK,
            LMASK & TMASK,
            RMASK & BMASK
        }, rev = 0;

    for (int i = 0; i < 8; i++) {
        unsigned long _rev = 0, trans = hand;

        while ((trans = transfer(trans, mask[i], dir[i])) && trans & *o) _rev |= trans;
        if (trans & *p) rev |= _rev;
    }

    *p ^= hand | rev;
    *o ^= rev;

    return rev;
}

/*
 * @brief Called when the game is finished.
 * @params black: black's bitboard
 *         white: white's bitboard
*/
void end(
    unsigned long black,
    unsigned long white
) {
    int
        bcount = countb(black),
        wcount = countb(white);

    puts("Game finished.");

    if      (wcount < bcount) puts("Winner: Black");
    else if (wcount > bcount) puts("Winner: White");
    else                      puts("Draw");
}

int main() {
    unsigned long 
        // 黒初期配置
        black = c2b(WIDTH / 2 - 1, HEIGHT / 2 - 1) | c2b(WIDTH / 2, HEIGHT / 2),
        // 白初期配置
        white = c2b(WIDTH / 2 - 1, HEIGHT / 2)     | c2b(WIDTH / 2, HEIGHT / 2 - 1),
        // 置ける場所
        placable = 0,
        // 手
        hand,
        // ひっくり返す
        rev = 0;


    int
        // 経過ターン(64から0)
        turn = WIDTH * HEIGHT,
        // パスしたかどうかのフラグ
        pflag = 0;

    // ずっとループ
    while (turn--) {
        // 黒のターン
        if (turn % 2) puts("\nBlack's turn\n"), placable = get_placable(black, white);
        // 白のターン
        else          puts("\nWhite's turn\n"), placable = get_placable(white, black);

        view(black, white, placable);

        // パス処理
        if (!placable) {
            pflag++;
            if (pflag == 1) {
                puts("Pass");
                continue;
            } else {
                view(black, white, placable);
                end(black, white);
                return 0;
            }
        } else pflag = 0;

        // 初回かどうか
        int is_first_time = 0;

        // 入力処理
        do {
            char x;
            int y;

            // 初回じゃなければ警告
            if (is_first_time) puts("Invaild Input.");
            else               is_first_time++;

            
            printf("Input<< ");
            scanf("%c%d", &x, &y);

            hand = c2b(x - 65, --y);
            scanf("%c", &x);
        } while (!(hand & placable)); // 有効手じゃない間ループ

        // 反転処理
        if (turn % 2) rev = flip(hand, &black, &white);
        else          rev = flip(hand, &white, &black);
    }
}