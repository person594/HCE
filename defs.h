#include <stdlib.h>

/*
8 56  57  58  59  60  61  62  63
7 48  49  50  51  52  53  54  55
6 40  41  42  43  44  45  46  47
5 32  33  34  35  36  37  38  39
4 24  25  26  27  28  29  30  31
3 16  17  18  19  20  21  22  23
2 08  09  10  11  12  13  14  15
1 00  01  02  03  04  05  06  07
  A   B   C   D   E   F   G   H
*/

enum {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
	NO_SQUARE = 64
};

#define R_1 0x00000000000000FFull
#define R_2 0x000000000000FF00ull
#define R_3 0x0000000000FF0000ull
#define R_4 0x00000000FF000000ull
#define R_5 0x000000FF00000000ull
#define R_6 0x0000FF0000000000ull
#define R_7 0x00FF000000000000ull
#define R_8 0xFF00000000000000ull

#define R_A 0x0101010101010101ull
#define R_B 0x0202020202020202ull
#define R_C 0x0404040404040404ull
#define R_D 0x0808080808080808ull
#define R_E 0x1010101010101010ull
#define R_F 0x2020202020202020ull
#define R_G 0x4040404040404040ull
#define R_H 0x8080808080808080ull

#define CENTER 0x0000001818000000ull

#define D_P 0x8040201008040201ull
#define D_N 0x0102040810204080ull

//bitboards for the moves of a knight and a king, each positioned at e5.
#define KNIGHTMOVE 0x0028440044280000ull
#define KINGMOVE 0x0000382838000000ull

#define C_WK 1
#define C_WQ 2
#define C_BK 4
#define C_BQ 8

#define P_VAL 100
#define N_VAL 300
#define B_VAL 300
#define R_VAL 500
#define Q_VAL 900
#define K_VAL 90000


typedef unsigned long long int bitboard;
//    0   1   2   3   4   5   6   7   8   9   10  11  12     13     14        15
enum {WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK, WHITE, BLACK, OCCUPIED, EMPTY};
extern int VAL[];
extern int ISBLACK[];
extern bitboard PDIAGS[15];
extern bitboard NDIAGS[15];

typedef struct s_board{
  bitboard bits[16];
  int ply;
  int enpas;
  int castle;
  
  int score; //white score - black score.
  
  //int pieces[12][10];
} Board;


#define BIT(pos) ((pos > 63 || pos < 0) ? 0ull : 1ull << pos)

#define RANK(pos) (R_1<<(pos&0x38))
#define ROW(pos) (R_A<<(pos&0x7))
#define PDIAG(pos) PDIAGS[7 + pos%8 - pos/8]
#define NDIAG(pos) NDIAGS[pos%8 + pos/8]


//macros to translate bitboards in a given direction
#define NORTH(b, n) (b<<(8*n))
#define SOUTH(b, n) (b>>(8*n))
//#define EAST(b) ((b & ~R_H)<<1)
//#define WEST(b) ((b & ~R_A)>>1)
#define EAST(b, n) ((b&((R_A<<(8-n)) - R_A))<<n)
#define WEST(b, n) ((b&~((R_A<<n) - R_A))>>n)
#define TRANS(b, x, y) (y >= 0 ? NORTH((x >= 0 ? EAST(b, x) : WEST(b, -x)), y) : SOUTH((x >= 0 ? EAST(b, x) : WEST(b, -x)), -y))

#define SAMERANK(p0, p1) !((p0^p1) >> 3)
#define SAMEROW(p0, p1) !((p0^p1) & 0x7)
/*
11100
11111
*/
#define RANKDIF(p0, p1) ((p0>>3) - (p1>>3))
#define ROWDIF(p0, p1) ((p0 & 0x7) - (p1 & 0x7))

#define ABS(n) (n >= 0 ? n : -n)

/* 0000	0000	0000	0000	pppp	tttt	ttff	ffff
t: to
f: from
p: pawn promotion piece
*/
typedef unsigned int move;
#define FROM(mov) mov & 0x3f
#define TO(mov) FROM(mov>>6)
#define PROM(mov) (mov >> 12) //& 0x0f
#define MOV(from, to, pawn) from|(to<<6)|(pawn<<12)

int pos(bitboard);
void printBitboard(bitboard);
void printBoard(Board);
void initBoard(Board*);
void clearPos(Board*, int);
void makeMove(Board*, move);
int sqAttacked(Board, int, int);

int popBit(bitboard*);
int countBits(bitboard); 

int diagslide(bitboard, int, int);
int orthslide(bitboard, int, int);

int validateBoardState(Board board);
int getPiece(char);
int getPos(char, char);
bitboard pieceMoves(Board, int, int);

int perftTest(Board, int);
int printMoves(Board);
void twoPlayerLoop(Board);
void onePlayerLoop(Board);
int getGameStatus(Board);

int getMoves(Board, int*);
int moveSearch(Board, int, int*);

int fromAlg(Board, char*);
void toAlg(Board, int, char*);
char getSymbol(int);
