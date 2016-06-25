#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#define USE_TABLE

#define TABLESIZE (1<<24)


//get a hash table key from a hash value
#define HASHKEY(n) (n % TABLESIZE)
#ifdef USE_TABLE
#define HASENTRY(n) (transpositionTable[HASHKEY(n)].hash == n)
#else
#define HASENTRY(n) 0
#endif




#define EXACT 0
#define LOWER 1
#define UPPER 2
#define BOOK 3 //for entries that only have an opening book move.  entries with opening book moves can have a type other than this, however

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
