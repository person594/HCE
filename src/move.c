#include "move.h"
#include "movegen.h"
#include "defs.h"


void removePiece(Position* pos, int sq){
	int p;
	bitboard b;
	p = pos->squares[sq];
	if (p == EMPTY) return;
	
	b = BIT(sq);
	pos->bits[p] &= ~b;
	pos->bits[COLOR[p]] &= ~b;
	pos->bits[OCCUPIED] &= ~b;
	pos->bits[EMPTY] |= b;
	pos->score -=VAL[p];
	pos->squares[sq] = EMPTY;
	pos->hash ^= PHASH(p, sq);
}

//only use when you know at compile time that sq is currently empty
void placePiece(Position *pos, int sq, int p) {
	bitboard b;
	if (p == EMPTY) return;
	b = BIT(sq);
	pos->bits[p] |= b;
	pos->bits[COLOR[p]] |= b;
	pos->bits[OCCUPIED] |= b;
	pos->bits[EMPTY] &= ~b;
	pos->score += VAL[p];
	pos->squares[sq] = p;
	pos->hash ^= PHASH(p, sq);
}

void setPiece(Position *pos, int sq, int p) {
	int p0;
	bitboard b;
	p0 = pos->squares[sq];
	if (p == p0) return;
	
	b = BIT(sq);
	pos->bits[p0] &= ~b;
	pos->bits[p] |= b;
	pos->bits[COLOR[p0]] &= ~b;
	pos->bits[COLOR[p]] |= b;
	pos->bits[OCCUPIED] =  ~(pos->bits[EMPTY]);
	pos->score +=VAL[p] - VAL[p0];
	pos->squares[sq] = p;
	if (p0 <= BK) {
		pos->hash ^= PHASH(p0, sq);
	}
	if (p <= BK) {
		pos->hash ^= PHASH(p, sq);
	}
}



int makeMove(Position* pos, int mov) {
	Position orig = *pos;
	int i, p, sq0, sq1, sd, prom, cast, enpas = NO_SQUARE;
	sq0 = FROM(mov);
	sq1 = TO(mov);
	sd = 6*(pos->ply%2);
	prom = PROM(mov) + sd;
	bitboard b0, b1;
	b0 = BIT(sq0);
	b1 = BIT(sq1);
	p = pos->squares[sq0];
	
	//update castling status from changes to the rooks.  changes to the kings are handled later.
	if ((sq0 == A1 || sq1 == A1) && (pos->castle & C_WQ)) {
		pos->castle &= (~C_WQ);
		pos->hash ^= CASTLEHASH(C_WQ);
	} else if ((sq0 == H1 || sq1 == H1) && (pos->castle & C_WK)) {
		pos->castle &= (~C_WK);
		pos->hash ^= CASTLEHASH(C_WK);
	} else if ((sq0 == A8 || sq1 == A8) && (pos->castle & C_BQ)) {
		pos->castle &= (~C_BQ);
		pos->hash ^= CASTLEHASH(C_BQ);
	} else if ((sq0 == H8 || sq1 == H8) && (pos->castle & C_BK)) {
		pos->castle &= (~C_BK);
		pos->hash ^= CASTLEHASH(C_BK);
	}
	
	removePiece(pos, sq0);
	setPiece(pos, sq1, p);
	
	switch (p) {	//special logic for special pieces
		int mask;
		case WP:
			if (sq1 - sq0 == 16){    //double ahead, set enpassant
				enpas = sq0+8;
			} else if (sq1  == pos->enpas){  //enpassant capture
				removePiece(pos, pos->enpas - 8);
			} else if (sq1/8 == 7){ //pawn promotion
				setPiece(pos, sq1, prom);
			}
			break;
		case BP:
			if (sq0 - sq1 == 16){		//double ahead, set enpassant
				enpas = sq1+8;
			} else if (sq1  == pos->enpas){		//enpassant capture
				removePiece(pos, pos->enpas + 8);
			} else if (sq1/8 == 0){	//pawn promotion
				//removePiece(pos, sq1);
				setPiece(pos, sq1, prom);
			}
			break;
			
		case WK:
		case BK:
			//prevent castling from side who moved their king
			mask = (p == WK) ? (C_WK | C_WQ) : (C_BK | C_BQ);
			int m2 = mask & pos->castle;
			int bit = C_WK;
			while (bit <= m2) {		//update our hash value.
				if (m2 & bit) {
					pos->hash ^= CASTLEHASH(bit);
				}
				bit <<= 1;
			}
			pos->castle &= (~mask);
			if (sq1 - sq0 == 2){	//king side castling
				removePiece(pos, sq1+1);
				placePiece(pos, sq1-1, WR + sd);
			} else if (sq0 - sq1 == 2){  //queen side castling
				removePiece(pos, sq1-2);
				placePiece(pos, sq1+1, WR + sd);
			}
			
	}
	pos->hash ^= pos->ephash;		//unapply the last ep hash;
	pos->ply++;										//increment pos ply, remember this needs to be updated before calling EPHASH
	pos->hash ^= WHITETURNHASH;
	pos->enpas = enpas;		//update en passant square
	pos->ephash = EPHASH(pos);	//calculate/store new ep has
	pos->hash ^= pos->ephash;   //apply new ep hash
	
	if (inCheck(pos, (sd == 0 ? WHITE : BLACK))) {
		unmakeMove(pos, mov);
		return 0;
	}
	return 1;
	//pos->hash ^= PHASH(p, sq0);
	//pos->hash ^= PHASH(p, sq1);
}


void unmakeMove(Position *pos, int mov) {
	int sq0, sq1, cap, prom, enpas, capSq, p, cast, sd;
	bitboard diff;
	
	sq0 = FROM(mov);
	sq1 = TO(mov);
	cap = CAP(mov);
	prom = PROM(mov);
	enpas = EP(mov);
	p = pos->squares[sq1];
	sd = ISBLACK[p] * 6;
	if (prom != EMPTY) p = WP + sd;
	cast = CAST(mov);
	capSq = sq1;
	if (enpas != NO_SQUARE) {
		if (sq1 == enpas && (p == WP || p == BP)) {
			capSq = (sq0 & 0x38) | (sq1 & 0x07); // same file as sq1, same rank as sq0
		}
	}
	diff = pos->castle ^ cast;
	while (diff) {
		pos->hash ^= CASTLEHASH(diff);
		popBit(&diff);
	}
	pos->castle = cast;
	removePiece(pos, sq1);
	placePiece(pos, sq0, p);
	placePiece(pos, capSq, cap);
	if ((p == WK || p == BK) && (sq0 - sq1 == 2 || sq0 - sq1 == -2)){ //castling: move the rook back
		removePiece(pos, (sq0 + sq1) / 2);
		if (sq1 > sq0) { //king side
			placePiece(pos, sq1 + 1, p - 2);
		} else { // queen side
			placePiece(pos, sq1 - 2, p - 2);
		}
	}
	
	pos->hash ^= pos->ephash;
	pos->ply--;
	pos->hash ^= WHITETURNHASH;
	pos->enpas = enpas;
	pos->ephash = EPHASH(pos);
	pos->hash ^= pos->ephash;
}
