#include <stdio.h>
#include <string.h>

#include "defs.h"
#include "movegen.h"
#include "position.h"
#include "transpositiontable.h"
#include "move.h"

extern int interrupt_flag;


//0: game in progress
//1: Checkmate, white wins
//2: Checkmate, black wins
//3: white in check
//4: black in check
//-1: Stalemate
int getGameStatus(Position *pos) {
	int moves[MAX_MOVES], nMoves, i;
	int sd;
	int check;
	sd = 6*(pos->ply%2);
	check = inCheck(pos, (sd == 0 ? WHITE : BLACK));
	nMoves = getMoves(pos, moves, 0, 0);
	for (i = 0; i < nMoves; ++i) {
		if (makeMove(pos, moves[i])) {
			unmakeMove(pos, moves[i]);
			return check * (3 + pos->ply%2);
		}
	}
	if (check) {
		return 2 - pos->ply%2;
	} else {
		return -1;
	}
}

int getMobility(Position *pos) {
	int p, n = 0;
	for (p = WP; p <= WK; ++p) {
		int sq;
		bitboard b;
		b = pos->bits[p];
		while ((sq = popBit(&b)) != NO_SQUARE) {
			n += countBits(pieceMoves(pos, p, sq));
		}
	}
	
	for (p = BP; p <= BK; ++p) {
		int sq;
		bitboard b;
		b = pos->bits[p];
		while ((sq = popBit(&b)) != NO_SQUARE) {
			n -= countBits(pieceMoves(pos, p, sq));
		}
	}
	return n;
	
}

int eval(Position *pos) {
	int sign;
	sign = 1 - 2*(pos->ply%2);
	//int pointsPerCenter = 35;
	//return pos->score + pointsPerCenter * (countBits(pos->bits[WHITE] & CENTER) - countBits(pos->bits[BLACK] & CENTER));
	//return sign * (pos->score + getMobility(pos));
	return sign*pos->score;
}
