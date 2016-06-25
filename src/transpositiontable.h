#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#define TABLESIZE (1<<24)

typedef struct {
	u64 hash;					//the full hashvalue of the position, as opposed to the index, which is of a smaller range
	int depth;				//the depth left when this position was encountered.
	int value;				//the value assigned to this
	int nodeType;
	int move;					//the previously determined best move
	int bookMove;
	int utility;
} tableEntry;

extern tableEntry transpositionTable[TABLESIZE];

void initTranspositionTable();

void clearTranspositionTable();

int addToTable(Position *pos, int score, int depth, int nodeType, int bestMove);

int getBookMove(Position *pos);
int getTableMove(Position *pos);
void getTableBounds(Position *pos, int *alpha, int *beta, int depth);

#endif
