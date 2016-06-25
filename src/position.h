#ifndef POSITION_H
#define POSITION_H

#include "bitboard.h"
#include "hash.h"

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
