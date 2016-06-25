#ifndef DEFS_H
#define DEFS_H

#include <stdlib.h>
#include <stdio.h>

#include "position.h"

#define SEARCH_DEPTH 9
#define MAX_MOVES 256


#define P_VAL 100
#define N_VAL 320
#define B_VAL 330
#define R_VAL 500
#define Q_VAL 900
#define K_VAL 90000

#define MAX_VAL (2*K_VAL)
#define MIN_VAL (-MAX_VAL)

//    0   1   2   3   4   5   6   7   8   9   10  11  12     13     14        15
enum {WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK, WHITE, BLACK, OCCUPIED, EMPTY};
extern int VAL[];
extern int ISBLACK[];
extern int COLOR[];

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
