#ifndef DEFS_H
#define DEFS_H

#include <stdlib.h>
#include <stdio.h>

#include "position.h"

#define SEARCH_DEPTH 9
#define MAX_MOVES 256
#define USE_TABLE


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

#define P_VAL 100
#define N_VAL 320
#define B_VAL 330
#define R_VAL 500
#define Q_VAL 900
#define K_VAL 90000

#define MAX_VAL (2*K_VAL)
#define MIN_VAL (-MAX_VAL)

#define EXACT 0
#define LOWER 1
#define UPPER 2
#define BOOK 3 //for entries that only have an opening book move.  entries with opening book moves can have a type other than this, however


//    0   1   2   3   4   5   6   7   8   9   10  11  12     13     14        15
enum {WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK, WHITE, BLACK, OCCUPIED, EMPTY};
extern int VAL[];
extern int ISBLACK[];
extern int COLOR[];
extern bitboard PDIAGS[15];
extern bitboard NDIAGS[15];
extern const u64 hashvals[781];
//extern tableEntry transpositionTable[TABLESIZE];


#define ABS(n) (n >= 0 ? n : -n)
/* 0eee  eeee  ssss  pppp  cccc  tttt  ttff  ffff
t: to
f: from
c: captured piece
p: pawn promotion piece
s: castling priviledges before move
e: en passant square before move
*/

#define FROM(mov) mov & 0x3f
#define TO(mov) (((mov)>>6) & 0x3f)
#define CAP(mov) (((mov) >> 12)  & 0x0f)
#define PROM(mov) (((mov) >> 16) & 0x0f)
#define CAST(mov) (((mov) >> 20 ) & 0x0f)
#define EP(mov) (((mov) >> 24) & 0x7f)
#define MOV(from, to, cap, prom, cast, ep) ((from)|((to)<<6)|((cap) << 12)|((prom)<<16)|((cast)<<20)|((ep)<<24))

//hash table things:
#define POLYGLOTPIECE(p) (2*((p)%6) + 1 - (p)/6)
#define PHASH(p, sq) (hashvals[64*POLYGLOTPIECE(p) + sq])
#define EPHASHRAW(sq) (hashvals[772+(sq)%8])
#define EPHASH(board) ((board)->enpas == NO_SQUARE ? 0ull : \
                      (board)->ply%2 ?      /*black to move*/ \
                      ((board)->bits[BP] & TRANS(0x5ull, (board)->enpas%8 - 1, 3) ? EPHASHRAW((board)->enpas) : 0ull) :\
                      ((board)->bits[WP] & TRANS(0x5ull, (board)->enpas%8 - 1, 4) ? EPHASHRAW((board)->enpas) : 0ull))
                      
//note this only works when given a single castling flag, not a set of flags
#define CASTLEHASH(flag) (hashvals[768 + (bsf(flag))])
#define WHITETURNHASH hashvals[780]

//get a hash table key from a hash value
#define HASHKEY(n) (n % TABLESIZE)
#ifdef USE_TABLE
#define HASENTRY(n) (transpositionTable[HASHKEY(n)].hash == n)
#else
#define HASENTRY(n) 0
#endif

int pos(bitboard);
void printBitboard(bitboard);
void printPosition(Position *);
int makeMove(Position *, int);
void unmakeMove(Position *, int);
int sqAttacked(Position *pos, int sq, int color);
int inCheck(Position *pos, int side);

int popBit(bitboard*);
int countBits(bitboard); 

int diagslide(bitboard, int, int);
int orthslide(bitboard, int, int);

int validatePosition(Position *pos);
int comparePositions(Position *b1, Position *b2);
int getPiece(char);
int getPos(char, char);

int perftTest(Position *, int);
int printMoves(Position *);
void twoPlayerLoop(Position *);
void onePlayerLoop(Position *);
int getGameStatus(Position *);

int getMoves(Position *pos, int moves[], int useBook, int onlyCaptures);
void orderMoves(Position *pos, int numMoves, int moves[]);
int alphaBetaMax(Position *, int, int, int, int *);
int alphaBetaMin(Position *, int, int, int, int *);
int alphaBeta(Position *pos, int alpha, int beta, int depthleft);
int moveSearch(Position *pos, int depth, int *score);
int addToTable(Position *pos, int score, int depth, int nodeType, int bestMove);
int getTableMove(Position *pos);
int getBookMove(Position *pos);
void getTableBounds(Position *pos, int *alpha, int *beta, int depth);

int eval(Position *pos);

int fromAlg(Position *, char*);
void toAlg(Position *, int, char*);
char getSymbol(int);

int getInputMove(Position *pos);

int bsf(bitboard b);
int bsr(bitboard b);
int sf(bitboard b);
int sr(bitboard b);


void readPolyglotBook(FILE *file);



typedef struct {
	char *name;
	int i_value;
	char *s_value;
	int accepted;
} feature;


char *readLine(void);

void xboardLoop(Position *pos);

#endif
