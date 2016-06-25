#include <string.h>

#include "defs.h"
#include "position.h"


void clearPosition(Position *pos) {
	int n, p;
	for (n = 0; n < 15; n++){
		pos->bits[n] = 0ull;
	}
	for (n = 0; n < 64; n++) {
		pos->squares[n] = EMPTY;
	}
	pos->bits[EMPTY] = ~pos->bits[OCCUPIED];
	pos->ply = 0;
	pos->enpas = NO_SQUARE;
	pos->castle  = C_WK | C_WQ | C_BK | C_BQ;
	
	pos->score = 0;
	pos->ephash = 0ull;
	pos->hash = hashPosition(pos);
}



void initPosition(Position* pos){
	int p, n;
	
	pos->ply = 0;
	pos->enpas = NO_SQUARE;
	pos->castle = C_WK | C_WQ | C_BK | C_BQ;
	
	pos->bits[WP] = R_2;
	pos->bits[WN] = R_1 & (R_B | R_G);
	pos->bits[WB] = R_1 & (R_C | R_F);
	pos->bits[WR] = R_1 & (R_A | R_H);
	pos->bits[WQ] = R_1 & R_D;
	pos->bits[WK] = R_1 & R_E;
	
	pos->bits[BP] = R_7;
	pos->bits[BN] = R_8 & (R_B | R_G);
	pos->bits[BB] = R_8 & (R_C | R_F);
	pos->bits[BR] = R_8 & (R_A | R_H);
	pos->bits[BQ] = R_8 & R_D;
	pos->bits[BK] = R_8 & R_E;
	
	pos->bits[WHITE] = R_1 | R_2;
	pos->bits[BLACK] = R_7 | R_8;
	pos->bits[OCCUPIED] = (pos->bits[WHITE]) | (pos->bits[BLACK]);
	pos->bits[EMPTY] = ~(pos->bits[OCCUPIED]);
	
	for (n = 0; n < 64; n++) {
		if (n/8 == 0 || n/8 == 7) {
			switch (n%8) {
				case 0:
				case 7:
					pos->squares[n] = WR;
					break;
				case 1:
				case 6:
					pos->squares[n] = WN;
					break;
				case 2:
				case 5:
					pos->squares[n] = WB;
					break;
				case 3:
					pos->squares[n] = WQ;
					break;
				case 4:
					pos->squares[n] = WK;
					break;
			}
		} else if (n/8 == 1 || n/8 == 6) {
			pos->squares[n] = WP;
		} else {
			pos->squares[n] = EMPTY;
		}
		if (n/8 >= 6) {
			pos->squares[n] += BP;
		}
	}
	
	pos->score = 0;
	pos->ephash = 0ull;
	pos->hash = hashPosition(pos);
}

//generates a position given Forsyth-Edwards Notation
int loadFEN(Position* pos, char* fen){
	int i, r, f;
	char *pieces, *side, *castling, *enpassant, *halfmove, *fullmove;
	char str[512];
	Position newPos; //make our position locally, then copy it over at the end
	strncpy(str, fen, 511);
	pieces = strtok(str, " ");
	side = strtok(NULL, " ");
	if (!side) return 0;
	castling = strtok(NULL, " ");
	if (!castling) return 0;
	enpassant = strtok(NULL, " ");
	if (!enpassant) return 0;
	halfmove = strtok(NULL, " ");
	if (!halfmove) return 0;
	fullmove = strtok(NULL, " ");
	if (!fullmove) return 0;
	clearPosition(&newPos);
	newPos.ply = 2 * (atol(fullmove) - 1);
	if (newPos.ply < 0) return 0;
	if (strcmp(side, "b") == 0) {
		newPos.ply++;
	} else if (strcmp(side, "w") != 0) {
		return 0;
	}
	newPos.castle = 0;
	if (strcmp(castling, "-") != 0) {
		if (*castling == 'K' || *castling == 'A') {
			newPos.castle |= C_WK;
			++castling;
		}
		if (*castling == 'Q' || *castling == 'H') {
			newPos.castle |= C_WQ;
			++castling;
		}
		if (*castling == 'k' || *castling == 'a') {
			newPos.castle |= C_BK;
			++castling;
		}
		if (*castling == 'q' || *castling == 'h') {
			newPos.castle |= C_BQ;
			++castling;
		}
		if (*castling) return 0;
	}
	if (strcmp(enpassant, "-") != 0) {
		int file, rank;
		file = enpassant[0] - 'a';
		if (file < 0 || file >= 8) return 0;
		rank = enpassant[1] - '1';
		if (rank != 2 && rank != 5) return 0;
		newPos.enpas = 8*rank + file;
	}
	newPos.ephash = EPHASH(&newPos);
	for (i = 0, r = 7, f = 0; pieces[i]; ++i) {
		int p;
		char ch;
		ch = pieces[i];
		if (ch == '/'){
			if (f != 8) return 0;
			if (--r < 0) return 0;
			f = 0;
		} else if (ch > '0' && ch <= '8'){
			f += (ch - '0');
		} else {
			if (f > 7) return 0;
			p = getPiece(ch);
			if (p == EMPTY) {
				return 0;
			}
			newPos.bits[p] |= BIT(r*8 + f);
			newPos.bits[OCCUPIED] |= BIT(r*8 + f);
			newPos.bits[WHITE + ISBLACK[p]] |= BIT(r*8 + f);
			newPos.score += VAL[p];
			newPos.squares[r*8 + f] = p;
			++f;
		}
	}
	if (r != 0 && f != 8) return 0;
	newPos.bits[EMPTY] = ~newPos.bits[OCCUPIED];
	newPos.hash = hashPosition(&newPos);
	*pos = newPos;
	return 1;
}

//This function is not fast.  It should be used to generate an initial hashcode for a new position.  If modifying an existing position, the board's hashcode should be modified on the fly.
u64 hashPosition(Position *pos) {
	u64 hash = 0ull;
	int sq;
	for (sq = 0; sq < 64; sq++) {
		if (pos->squares[sq] != EMPTY) {
			hash ^= PHASH(pos->squares[sq], sq);
		}
	}
	//en passant
	hash ^= EPHASH(pos);
	//castling
	int castle;
	for (castle = 8; castle; castle >>= 1) {
		if (castle & pos->castle) {
			hash ^= CASTLEHASH(castle);
		}
	}
	//ply
	if (pos->ply%2 == 0) {
		hash ^= WHITETURNHASH;
	}
	return hash;
}
