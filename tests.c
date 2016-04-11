#include <stdio.h>

#include "defs.h"


int perftTest(Board *board, int depth){
	int sd, i, nMoves, sum = 0;
	int moves[MAX_MOVES];
	sd = (board->ply % 2) * 6;
	if (sqAttacked(board, bsf(board->bits[BK - sd]), WHITE + (board->ply % 2))){
		return 0;
	}
	if (depth == 0) {
		return 1;
	}
	
	nMoves = getMoves(board, moves, 0);
	for (i = 0; i < nMoves; ++i) {
		if (makeMove(board, moves[i])) {
			sum += perftTest(board, depth - 1);
			unmakeMove(board, moves[i]);
		}
	}
	return sum;
}
