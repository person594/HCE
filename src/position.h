#ifndef POSITION_H
#define POSITION_H

#include "bitboard.h"
#include "hash.h"

typedef struct {
  bitboard bits[16];
  int ply;
  int enpas;
  int castle;
  int score; //white score - black score.
  int squares[64];
  u64 hash;
  u64 ephash;		//last calculated en passant hash value, used for unapplying it.
  //int entropy; //number of non-reversible events which have occurred, such as pawn advances, captures, and castling rights lost
  //             //used for determining when to replace transposition table entries 
} Position;

#define C_WK 1
#define C_WQ 2
#define C_BK 4
#define C_BQ 8

void clearPosition(Position *pos);
void initPosition(Position *pos);
int loadFEN(Position* pos, char* fen);
u64 hashPosition(Position *pos);


#endif
