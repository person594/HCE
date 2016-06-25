#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "defs.h"

#define MAX_MOVES 256


int diagslide(bitboard, int, int);
int orthslide(bitboard, int, int);

bitboard pieceMoves(Position *pos, int p, int sq);
bitboard pieceCaptures(Position *pos, int p, int sq);

int getMoves(Position *pos, int moves[], int useBook, int onlyCaptures);
void orderMoves(Position *pos, int numMoves, int moves[]);

int sqAttacked(Position *pos, int sq, int side);
int inCheck(Position *pos, int side);

int perftTest(Position *pos, int ply);

#endif
