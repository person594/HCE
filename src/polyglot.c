#include <stdio.h>
#include "defs.h"

int entries = 0;
int collisions = 0;

//reads a hash value from the file, taking care of endianness
int readHash(FILE *file, u64 *hash) {
	unsigned char bytes[8];
	fread(bytes, 8, 1, file);
	*hash =  
	    ((u64)bytes[0] << 56)
		| ((u64)bytes[1] << 48)
		| ((u64)bytes[2] << 40)
		| ((u64)bytes[3] << 32)
		| ((u64)bytes[4] << 24)
		| ((u64)bytes[5] << 16)
		| ((u64)bytes[6] << 8)
		| ((u64)bytes[7]);
		return !feof(file);
}

int fromPolyglotMove(int polyglotMove) {
	int f0, f1, r0, r1;
	int sq0, sq1;
	int p, prom;
	f1 = polyglotMove % 8;
	polyglotMove /= 8;
	r1 = polyglotMove % 8;
	polyglotMove /= 8;
	f0 = polyglotMove % 8;
	polyglotMove /= 8;
	r0 = polyglotMove % 8;
	polyglotMove /= 8;
	switch (polyglotMove % 8) {
		case 0:
			prom = EMPTY;
			break;
		case 1:
			prom = WN;
			break;
		case 2:
			prom = WB;
			break;
		case 3:
			prom = WR;
			break;
		case 4:
			prom = WQ;
			break;
	}
	
	sq0 = 8*r0 + f0;
	sq1 = 8*r1 + f1;
	return MOV(sq0, sq1, 0, prom, 0, 0);
}

int readPolyglotEntry(FILE *file) {
	u64 hash;
	if (readHash(file, &hash)) {
		int move, weight;
		unsigned long learn;
		tableEntry *t;
		
		++entries;
		
		move = fgetc(file) << 8;
		move |= fgetc(file);
		weight = fgetc(file) << 8;
		weight |= fgetc(file);
		learn = fgetc(file) << 24;
		learn |= fgetc(file) << 16;
		learn |= fgetc(file) << 8;
		learn |= fgetc(file);
		
		t = &transpositionTable[HASHKEY(hash)];
		if (t->hash % TABLESIZE == HASHKEY(hash)) {
			if (t->hash != hash ) {
				++collisions;
			}
			if (t->value > weight) {
				return 1;
			}
		}
		t->hash = hash;
		t->nodeType = BOOK;
		t->value = weight; //we are misusing this field to remember which move was the best when the book contains multiple moves for a position
		                   //this will be set to its proper value as the program runs
		t->bookMove = fromPolyglotMove(move);
		return 1;
	} else return 0;
}



void readPolyglotBook(FILE *file) {
	int i;
	while (readPolyglotEntry(file));
}
