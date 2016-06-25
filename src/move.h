#ifndef MOVE_H
#define MOVE_H

#include "position.h"



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

void removePiece(Position* pos, int sq);
void placePiece(Position *pos, int sq, int p);
void setPiece(Position *pos, int sq, int p);

int makeMove(Position *, int);
void unmakeMove(Position *, int);

#endif
