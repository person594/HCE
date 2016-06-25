#include <stdio.h>

#include "defs.h"
#include "transpositiontable.h"
#include "movegen.h"
#include "move.h"

//a random number for each combination of piece and square.  Squares 0-7 on the WP array represent en-passant column. castling status is stored on squares 0, 1, 3, and 7 of the BP array.
//Black's turn is square 2 of the BP array.
tableEntry transpositionTable[TABLESIZE];


void initTranspositionTable() {
	static int initiated = 0;
	if (!initiated) {
		clearTranspositionTable();
		initiated = 1;
	}
}

//clears the table by setting each hashvalue to an impossible value for the position.
void clearTranspositionTable() {
	int i;
	for (i = 0; i < TABLESIZE; i++) {
		transpositionTable[i].move = 0;
		transpositionTable[i].bookMove = 0;
		if (transpositionTable[i].hash % TABLESIZE == i) {
			transpositionTable[i].hash++;
		}
	}
}

int addToTable(Position *pos, int score, int depth, int nodeType, int bestMove) {
	tableEntry *t;
	int i;
	i = HASHKEY(pos->hash);
	t = &transpositionTable[i];
	if (t->hash == pos->hash) { //previously seen position
		if (!(bestMove && ! t->move) && t->depth >= depth) return 0;
	} else if (HASHKEY(t->hash) == i) { //type-2 error
			if (0) return 0; //todo -- add replacement policy
	}
	t->hash = pos->hash;
	t->depth = depth;
	t->value = score;
	t->nodeType = nodeType;
	t->move = bestMove;
	t->utility = 0;
	return 1;
}

int getBookMove(Position *pos) {
	tableEntry *t;
	int i;
	i = HASHKEY(pos->hash);
	t = &transpositionTable[i];
	if (t->hash == pos->hash && t->bookMove) {
		//the opening book doesn't contain information like en-passant
		//and castling rights, so we must provide it
		int move;
		int p, sq0, sq1, cap, prom;
		int sd;
		sd = 6 * (pos->ply % 2);
		move = t->bookMove;
		sq0 = FROM(move);
		sq1 = TO(move);
		p = pos->squares[sq0];
		cap = pos->squares[sq1];
		if ((p == WP || p == BP) && cap == EMPTY && sq1%8 != sq0%8) { //en passant
			cap = BP - sd;
		}
		prom = PROM(move);
		return MOV(sq0, sq1, cap, prom, pos->castle, pos->enpas);
	}
	return 0;
}

int getTableMove(Position *pos) {
	tableEntry *t;
	int i;
	i = HASHKEY(pos->hash);
	t = &transpositionTable[i];
	if (t->hash == pos->hash && t->nodeType != BOOK) {
		++t->utility;
		return t->move;
	} else {
		--t->utility;
		return 0;
	}
}

void getTableBounds(Position *pos, int *alpha, int *beta, int depth) {
	tableEntry *t;
	int i;
	i = HASHKEY(pos->hash);
	t = &transpositionTable[i];
	if (t->hash == pos->hash) {
		if (t->depth >= depth) {
			switch(t->nodeType) {
				case EXACT:
					*alpha = *beta = t->value;
					break;
				case LOWER:
					if (*alpha < t->value) {
						*alpha = t->value;
					}
					break;
				case UPPER:
					if (*beta > t->value) {
						*beta = t->value;
					}
					break;
			}
		}
	}
}
