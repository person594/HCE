#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "defs.h"


bitboard pieceMoves(Position *pos, int p, int sq);
bitboard pieceCaptures(Position *pos, int p, int sq);

int getMoves(Position *pos, int* moves, int useBook, int onlyCaptures);

#endif
