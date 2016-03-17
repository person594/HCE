#include "defs.h"
#include <stdio.h>

//note use of xor instead of or to facilitate randomness whenever RAND_MAX >= 2^15-1
#define RAND64 (u64)rand() | \
               (u64)rand() << 15 ^ \
               (u64)rand() << 30 ^ \
               (u64)rand() << 45 ^ \
               (u64)rand() << 60

//a random number for each combination of piece and square.  Squares 0-7 on the WP array represent en-passant column. castling status is stored on squares 0, 1, 3, and 7 of the BP array.
//Black's turn is square 2 of the BP array.
tableEntry transpositionTable[TABLESIZE];

//clears the table by setting each hashvalue to an impossible value for the position.
void clearTranspositionTable() {
	int i;
	for (i = 0; i < TABLESIZE; i++) {
		transpositionTable[i].move = -1;
		if (transpositionTable[i].hash % TABLESIZE == i) {
			transpositionTable[i].hash++;
		}
	}
}

void initHashTable() {
	static int initiated = 0;
	if (!initiated) {
		clearTranspositionTable();
		initiated = 1;
	}
}

/*  I need to modify my representation of castle moves and pawn promotion pieces before implementing this.
int loadOpeningBook(char *fileName) {
	FILE *fp = fopen(fileName, "r");
	if (!fp) {
		return -1;
	}
	int entries = 0;
	while (!feof(fp)) {
		tableEntry entry;
		short polymov = 0; //the polyglot-format move
		//get the hash value
		fread(&entry.hash, 64, 1, fp);
		//get the move
		fread(&pmove, 16, 1, fp);
		//convert the move to a sensible format
		entry.move = MOV(pmov&0x3f, (pmov>>6)&0x3f, pmov>>12)
	}
}
*/

//This function is not fast.  It should be used to generate an initial hashcode for a new board.  If modifying an existing board, the board's hashcode should be modified on the fly.
u64 getHashCode(Board *board) {
	u64 hash = 0ull;
	int sq;
	for (sq = 0; sq < 64; sq++) {
		if (board->squares[sq] != EMPTY) {
			hash ^= PHASH(board->squares[sq], sq);
		}
	}
	//en passant
	hash ^= EPHASH(board);
	//castling
	int castle;
	for (castle = 8; castle; castle >>= 1) {
		if (castle & board->castle) {
			hash ^= CASTLEHASH(castle);
		}
	}
	//ply
	if (board->ply%2 == 0) {
		hash ^= WHITETURNHASH;
	}
	return hash;
}
