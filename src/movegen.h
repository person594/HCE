#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "defs.h"


bitboard pieceMoves(Board *board, int p, int sq);
bitboard pieceCaptures(Board *board, int p, int sq);

int getMoves(Board *board, int* moves, int useBook, int onlyCaptures);

#endif
