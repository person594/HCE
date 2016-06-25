#include <stdio.h>
#include <string.h>

#include "defs.h"
#include "movegen.h"
#include "position.h"
#include "transpositiontable.h"

extern int interrupt_flag;

void printBitboard(bitboard bits){
  int r, c;
  for (r = 7; r >= 0; r--){
  	for (c = 0; c < 8; c++){
  		int i = r*8 + c;
  		char c = ((bits >> i) & 1) ? 'x': '.';
  		printf("%c", c);
  	}
  	printf("\n");
  }
}

void printPosition(Position *pos){
	char chars[64];
	int p, n, r, c;
	for (n = 0; n < 64; n++){
		chars[n] = ' ';
	}
	for (p = 0; p < 12; p++) {
		int sq;
		bitboard b = pos->bits[p];
		while ((sq = popBit(&b)) != NO_SQUARE) {
			chars[sq] = getSymbol(p);
		}
	}
	/* en passant square
	if (pos->enpas != NO_SQUARE) {
		chars[pos->enpas] = 'x';
	}
	*/
	//printf("%.8s\n%.8s\n%.8s\n%.8s\n%.8s\n%.8s\n%.8s\n%.8s\n", &chars[56], &chars[48], &chars[40], &chars[32], &chars[24], &chars[16], &chars[8], &chars[0]);
	for (r = 7; r >= 0; r--) {
		for (c = 0; c < 8; c++) {
			int at = 1, fg = 31, bg;
			if (r%2 == c%2) {
				bg = 45;
			} else {
				bg = 47;
			}
			//fg = 77 - bg;
			printf("%c[%d;%d;%dm",27,at,fg,bg);
			printf("%c ", chars[8*r + c]);
		}
		printf("%c[0m", 27);
		printf("\n");
	}
	//printf("%llx\n", pos->hash);
}

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

int diagslide(bitboard occupied, int p0, int p1) {
	int delta, p;
	//quickly exit if the two positions are not diagonal
	if (ABS(FILEDIF(p0, p1)) != ABS(RANKDIF(p0, p1))){
		return 0;
	}
	if (p0 == p1){	//pieces don't attack their own position.
		return 0;
	}
	if (p0 < p1){ //positive delta
		delta = ((p1 - p0) % 7) ? 9 : 7;
	} else {			//negative delta
		delta = ((p0 - p1) % 7) ? -9 : -7;
	}
	for (p = p0 + delta; p != p1; p+=delta){
		if (occupied & BIT(p)){
			return 0;
		}
	}
	return 1;
}

int orthslide(bitboard occupied, int p0, int p1) {
	int delta, p;
	if (SAMERANK(p0, p1) == SAMEFILE(p0, p1)){	//ensure p0 and p1 share either a row or rank, not both or neither
		return 0;
	}
	if (p1 > p0){
		delta = SAMERANK(p0, p1) ? 1 : 8;
	} else {
		delta = SAMERANK(p0, p1) ? -1 : -8;
	}
	for (p = p0 + delta; p != p1; p+=delta){
		if (occupied & BIT(p)){
			return 0;
		}
		
	}
	return 1;
}

/*
checks if side is attacking sq
*/
int sqAttacked(Position *pos, int sq, int side) {
	bitboard attackers;
	int aSd, dSd; //sd of the (maybe hypothetical) piece on the square being attacked
	int pawnDelta;
	aSd = (side == WHITE) ? 0 : 6;
	dSd = 6 - aSd;
	pawnDelta = (side == WHITE) ? -8 : 8;
	//reorder these based on commonness
	//pawn
	attackers = BIT(sq+pawnDelta - 1) | BIT(sq + pawnDelta + 1);
	attackers &= RANKOF(sq + pawnDelta);
	if (attackers & pos->bits[WP+aSd]) return 1;
	//knight
	attackers = TRANS(KNIGHTMOVE, FILEDIF(sq,E5), RANKDIF(sq, E5));
	if (attackers & pos->bits[WN+aSd]) return 1;
	//bishop and queen diagonal
	attackers = pieceCaptures(pos, WB+dSd, sq);
	if (attackers & (pos->bits[WB+aSd] | pos->bits[WQ+aSd]) ) return 1;
	//rook and queen orthogonal
	attackers = pieceCaptures(pos, WR+dSd, sq);
	if (attackers & (pos->bits[WR+aSd] | pos->bits[WQ+aSd]) ) return 1;
	//king
	attackers = pieceCaptures(pos, WK+dSd, sq);
	if (attackers & pos->bits[WK+aSd]) return 1;
	return 0;
	
	
	
	#if 0
	int sd, i;
	sd = (side == WHITE) ? 0 : 6;
	
	for (i = 0; i < 6; i++) {
		int n, p, sq0;
		bitboard b;
		p = sd + i;
		b = pos->bits[p];
		while ((sq0 = popBit(&b)) != NO_SQUARE) {
			/*
			switch over each piece type and check if it is threatening the given square.
			we need not check the piece's color, as we are only looping over p values of the proper color.
			*/
			switch (p) {
				case WP:
					if (side == WHITE && RANKDIF(sq, sq0) == 1 && (FILEDIF(sq, sq0) == 1 || FILEDIF(sq, sq0) == -1)){
						return 1;
					}
					break;
				case BP:
					if (side == BLACK && RANKDIF(sq, sq0) == -1 && (FILEDIF(sq, sq0) == 1 || FILEDIF(sq, sq0) == -1)){
						return 1;
					}
					break;
				case WN:
				case BN:
					if ((ABS(RANKDIF(sq, sq0)) == 2 && ABS(FILEDIF(sq, sq0)) == 1) || (ABS(RANKDIF(sq, sq0)) == 1 && ABS(FILEDIF(sq, sq0)) == 2)){
						return 1;
					}
					break;
				case WB:
				case BB:
					if (diagslide(pos->bits[OCCUPIED], sq0, sq)){
						return 1;
					}
					break;
				case WR:
				case BR:
					if (orthslide(pos->bits[OCCUPIED], sq0, sq)){
						return 1;
					}
					break;
				case WQ:
				case BQ:
					if (diagslide(pos->bits[OCCUPIED], sq0, sq) || orthslide(pos->bits[OCCUPIED], sq0, sq)){
						return 1;
					}
					break;
				case WK:
				case BK:
					if (ABS(RANKDIF(sq0, sq)) <= 1 && ABS(FILEDIF(sq0, sq)) <= 1 && sq0 != sq){
						return 1;
					}
			}
		}
	}
	return 0;
	#endif
}

int inCheck(Position *pos, int side) {
	int sd, attacker;
	sd = (side == WHITE ? 0 : 6);
	attacker = (side == WHITE ? BLACK : WHITE);
	return sqAttacked(pos, bsf(pos->bits[WK+sd]), attacker);
}




inline int countBits(bitboard b) {
	u64 c;
	asm (
		"popcntq %1, %0"
		: "=r" (c)
		: "r" (b)
	);
	return c;
}

/*
given a bitboard, returns the first occupied position
*/
inline int bsf(bitboard b){
	unsigned long long int v;
	asm (
		"bsfq %1, %0\n\t"
		"jnz 1f\n\t"
		"movq $64, %0\n\t"
		"1:"
		: "=r" (v)
		: "r" (b)
	);
	return (int) v;
}

inline int bsr(bitboard b){
	unsigned long long int v;
	asm (
		"bsrq %1, %0\n\t"
		"jnz 1f\n\t"
		"movq $64, %0\n\t"
		"1:"
		: "=r" (v)
		: "r" (b)
	);
	return (int) v;
}

//like bsf, but returns 63 if a 1 is not found.
inline int sf(bitboard b){
	unsigned long long int v = 63;
	asm (
		"bsfq %1, %0\n\t"
		"jnz 1f\n\t"
		"movq $63, %0\n\t"
		"1:"
		: "=r" (v)
		: "r" (b)
	);
	return v;
}

//like bsr, but returns 0 if a 1 is not found
inline int sr(bitboard b){
	unsigned long long int v = 0;
	asm (
		"bsrq %1, %0\n\t"
		"jnz 1f\n\t"
		"movq $0, %0\n\t"
		"1:"
		: "=r" (v)
		: "r" (b)
	);
	return v;
}

inline int popBit(bitboard* b){
	int p = bsf(*b);
	*b &= *b - 1;
	return p;
}

int validatePosition(Position *pos) {
	int p, n, squares[64], score = 0;
	bitboard occupied = 0, white = 0, black = 0;
	
	for (n = 0; n < 64; n++) {
		squares[n] = EMPTY;
	}
	
	for (p = WP; p <= BK; p++){
		bitboard pbits;
		pbits = pos->bits[p];
		if (occupied & pbits) {
			printf("Multiple pieces at a single square.\n");
			return 0;
		}
		occupied |= pbits;
		if (p < BP) {
			white |= pbits;
		} else {
			black |= pbits;
		}
		score += VAL[p] * countBits(pbits);
		while (pbits) {
			squares[popBit(&pbits)] = p;
		}
		
	}
	//generate the rest of our bitboards:
	
	if (white != pos->bits[WHITE] || black != pos->bits[BLACK] || occupied != pos->bits[OCCUPIED] || ~occupied != pos->bits[EMPTY]) {
		printf("Incorrect derived bitboards.\n");
		printf("white:\n");
		printBitboard(pos->bits[WHITE]);
		printf("\nblack:\n");
		printBitboard(pos->bits[BLACK]);
		printf("\noccupied:\n");
		printBitboard(pos->bits[OCCUPIED]);
		printf("\nempty:\n");
		printBitboard(pos->bits[EMPTY]);
		return 0;
	}
	
	if (score != pos->score) {
		printf("Incorrect score, expected: %d, actual: %d\n", score, pos->score);
		return 0;
	}
	
	for (n = 0; n < 64; n++) {
		if (squares[n] != pos->squares[n]) {
			printf("Incorrect piece buffer.");
			return 0;
		}
	}
	if (pos->ephash != EPHASH(pos)) {
		printf("Incorrect en passant hash code, expected: %llx, actual: %llx.\n", EPHASH(pos), pos->ephash);
		return 0;
	}
	
	if (pos->hash != hashPosition(pos)) {
		printf("Incorrect hash code, expected: %llx, actual: %llx.\n", hashPosition(pos), pos->hash);
		return 0;
	}
	
	return 1;
}

int comparePositions(Position *pos1, Position *pos2) {
	int p, sq;
	for (p = 0; p < 15; ++p) {
		if (pos1->bits[p] !=pos2->bits[p]) {
			printf("Bitboard mismatch for piece %d\n", p);
			printBitboard(pos1->bits[p]);
			printf("\n");
			printBitboard(pos2->bits[p]);
			printf("\n");
			return 0;
		}
	}
	if (pos1->ply != pos2->ply) {
		printf("Ply mismatch\n%d\n%d\n", pos1->ply, pos2->ply);
		return 0;
	}
	
	if (pos1->enpas != pos2->enpas) {
		printf("En passant square mismatch\n%d\n%d\n", pos1->enpas, pos2->enpas);
		return 0;
	}
	
	if (pos1->castle != pos2->castle) {
		printf("Castling rights mismatch\n%x\n%x\n", pos1->castle, pos2->castle);
		return 0;
	}
  if (pos1->score != pos2->score) {
		printf("Score mismatch\n%d\n%d\n", pos1->score, pos2->score);
		return 0;
	}
	for (sq = 0; sq < 64; ++sq) {
		if (pos1->squares[sq] != pos2->squares[sq]) {
			printf("Square mismatch\n");
			printPosition(pos1);
			printPosition(pos2);
			return 0;
		}
	}
	if (pos1->hash != pos2->hash) {
		printf("Hash mismatch\n%x\n%x\n", pos1->hash, pos2->hash);
		return 0;
	}
	if (pos1->ephash != pos2->ephash) {
		printf("En passant hash mismatch\n%x\n%x\n", pos1->hash, pos2->hash);
		return 0;
	}
	return 1;
}

int getPos(char row, char rank){
	row |= 0x20;	//to lower case
	if (row < 'a' || row > 'h' || rank < '1' || rank > '8'){
		return NO_SQUARE;
	}
	return 8*(rank - '1') + (row - 'a');
}

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

void quicksortMoves(int n, int moves[], int scores[]) {
	int tmp;
	int pivot, p, i, parity = 0;
	#define SWAP(a, b) (tmp = scores[(a)], scores[(a)] = scores[(b)], scores[(b)] = tmp, tmp = moves[(a)], moves[(a)] = moves[(b)], moves[(b)] = tmp)
	#define CSWAP(a, b) (scores[(a)] < scores[(b)] ? (SWAP((a), (b))) : 0)
	//first try to use a sorting network if we know one for numMoves
	switch(n) {
		case 0:
		case 1:
			return;
		case 2:
			CSWAP(0, 1);
			return;
		case 3:
			CSWAP(0, 1);
			CSWAP(0, 2);
			CSWAP(1, 2);
			return;
		case 4:
			CSWAP(0, 1);
			CSWAP(2, 3);
			CSWAP(0, 2);
			CSWAP(1, 3);
			CSWAP(1, 2);
			return;
		case 5:
			CSWAP(0, 1);
			CSWAP(2, 3);
			CSWAP(0, 2);
			CSWAP(1, 4);
			CSWAP(0, 1);
			CSWAP(2, 3);
			CSWAP(1, 2);
			CSWAP(3, 4);
			CSWAP(2, 3);
			return;
		case 6:
			CSWAP(0, 1);
			CSWAP(2, 3);
			CSWAP(4, 5);
			CSWAP(0, 2);
			CSWAP(1, 4);
			CSWAP(3, 5);
			CSWAP(0, 1);
			CSWAP(2, 3);
			CSWAP(4, 5);
			CSWAP(1, 2);
			CSWAP(3, 4);
			CSWAP(2, 3);
			return;
		default:
			//partition
			pivot = scores[n-1];
			for (p = i = 0; i < n-1; ++i) {
				//parity is a hack to try to make the pivot near the middle of the array when many values == pivot
				if (scores[i] == pivot) {
					parity = 1 - parity;
				}
				if (scores[i] >= pivot + parity) {
					SWAP(p, i);
					++p;
				}
			}
			SWAP(p, n-1);
			//recurse
			quicksortMoves(p, moves, scores);
			quicksortMoves(n - p - 1, &moves[p+1], &scores[p+1]);
			return;
	}
	#undef SWAP
	#undef CSWAP
}

void orderMoves(Position *pos, int numMoves, int moves[]) {
	int tmp;
	int i;
	int scores[MAX_MOVES];
	int tableMove;
	tableMove = getTableMove(pos);
	if (numMoves < 2) return;
	for (i = 0; i < numMoves; ++i) {
		if (moves[i] == tableMove) {
			scores[i] = MAX_VAL;
		} else {
			/*
			if (makeMove(pos, moves[i])) {
				scores[i] = -eval(pos);
				unmakeMove(pos, moves[i]);
			} else {
				scores[i] = MIN_VAL;
			}
			*/
			scores[i] = ABS(VAL[CAP(moves[i])]);
		}
	}
	quicksortMoves(numMoves, moves, scores);
	return;
}

int eval(Position *pos) {
	int sign;
	sign = 1 - 2*(pos->ply%2);
	//int pointsPerCenter = 35;
	//return pos->score + pointsPerCenter * (countBits(pos->bits[WHITE] & CENTER) - countBits(pos->bits[BLACK] & CENTER));
	//return sign * (pos->score + getMobility(pos));
	return sign*pos->score;
}

int quiescence(Position *pos, int alpha, int beta) {
	int numMoves, i, moves[MAX_MOVES], bestMove = 0;
	int nodeType = UPPER;
	int standingPat;
	#define DELTA 200
	standingPat = eval(pos); 
	if (standingPat >= beta) {
		return beta;
	}
	if (standingPat > alpha) {  //null move heuristic
		alpha = standingPat;
	}
	if (standingPat + Q_VAL < alpha) { //futility check
		return alpha;
	}
	if (!pos->bits[WK] || !pos->bits[BK]) {
		return standingPat;
	}
	getTableBounds(pos, &alpha, &beta, 0);
	if (alpha >= beta) return alpha;
	numMoves = getMoves(pos, moves, 1, 1);
	orderMoves(pos, numMoves, moves);

	for (i = 0; i < numMoves; ++i) {
		int score;
		//delta pruning
		if (ABS(VAL[CAP(moves[i])]) + DELTA < alpha) continue;
		if (makeMove(pos, moves[i])) {
			score = -quiescence(pos, -beta, -alpha);
			unmakeMove(pos, moves[i]);
			if (score >= beta) {
				nodeType = LOWER;
				addToTable(pos, beta, 0, nodeType, moves[i]);
				return beta;
			}
			if (score > alpha) {
				alpha = score;
				bestMove = moves[i];
			}
		}
	}
	addToTable(pos, beta, 0, nodeType, bestMove);
	return alpha;
}

int moveSearch(Position *pos, int depth, int *score) {
	int numMoves, i, moves[MAX_MOVES];
	int bestMove;
	int current_depth;
	interrupt_flag = 0;
	for (current_depth = 0; current_depth < depth; ++current_depth) {
		int alpha = MIN_VAL;
		numMoves = getMoves(pos, moves, 1, 0);
		orderMoves(pos, numMoves, moves);
		for (i = 0; i < numMoves; ++i) {
			int score;
			if (makeMove(pos, moves[i])) {
				score = -alphaBeta(pos, MIN_VAL, -alpha, current_depth);
				unmakeMove(pos, moves[i]);
				if (score > alpha) {
					alpha = score;
					bestMove = moves[i];
				}
			}
		}
	}
	return bestMove;
}

int alphaBeta(Position *pos, int alpha, int beta, int depthleft) {
	int numMoves, i, moves[MAX_MOVES];
	int nodeType = UPPER;
	int bestMove = 0;
	int legalMove = 0;
	getTableBounds(pos, &alpha, &beta, depthleft);
	if (interrupt_flag) return alpha;
	if (alpha >= beta) return alpha;
	if (depthleft <= 0 || !pos->bits[WK] || !pos->bits[BK]) {
		return quiescence(pos, alpha, beta);
	}
	numMoves = getMoves(pos, moves, 1, 0);
	orderMoves(pos, numMoves, moves);
	for (i = 0; i < numMoves; ++i) {
		int score;
		if (makeMove(pos, moves[i])) {
			legalMove = 1;
			score = -alphaBeta(pos, -beta, -alpha, depthleft - 1);
			unmakeMove(pos, moves[i]);
			if (score >= beta) {
				nodeType = LOWER;
				addToTable(pos, beta, depthleft, nodeType, moves[i]);
				return beta;
			}
			if (score > alpha) {
				alpha = score;
				bestMove = moves[i];
				nodeType = EXACT;
			}
		}
	}
	if (!legalMove) {
		if (inCheck(pos, (pos->ply%2 == 0 ? WHITE : BLACK))) return alpha;
		else return 0;
	}
	addToTable(pos, alpha, depthleft, nodeType, bestMove);
	return alpha;
}


