#include <stdio.h>

#include "defs.h"



int perftTest(Position *pos, int depth){
	int i, nMoves, sum = 0;
	int moves[MAX_MOVES];
	if (depth == 0) {
		return 1;
	}
	
	nMoves = getMoves(pos, moves, 0, 0);
	for (i = 0; i < nMoves; ++i) {
		if (makeMove(pos, moves[i])) {
			sum += perftTest(pos, depth - 1);
			unmakeMove(pos, moves[i]);
		}
	}
	return sum;
}
