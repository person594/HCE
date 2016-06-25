#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"

#define SEARCH_DEPTH 9

int alphaBeta(Position *pos, int alpha, int beta, int depthleft);
int quiescence(Position *pos, int alpha, int beta);
int moveSearch(Position *pos, int depth, int *score);


#endif
