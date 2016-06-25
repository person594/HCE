#include <stdio.h>
#include <string.h>

#include "defs.h"
#include "movegen.h"

extern int interrupt_flag;

void printBitboard(bitboard board){
  int r, c;
  for (r = 7; r >= 0; r--){
  	for (c = 0; c < 8; c++){
  		int i = r*8 + c;
  		char c = ((board >> i) & 1) ? 'x': '.';
  		printf("%c", c);
  	}
  	printf("\n");
  }
}

void printBoard(Board *board){
	char chars[64];
	int p, n, r, c;
	for (n = 0; n < 64; n++){
		chars[n] = ' ';
	}
	for (p = 0; p < 12; p++) {
		int pos;
		bitboard b = board->bits[p];
		while ((pos = popBit(&b)) != NO_SQUARE) {
			chars[pos] = getSymbol(p);
		}
	}
	/* en passant square
	if (board->enpas != NO_SQUARE) {
		chars[board->enpas] = 'x';
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
	//printf("%llx\n", board->hash);
}

void clearBoard(Board* board) {
	int n, p;
	for (n = 0; n < 15; n++){
		board->bits[n] = 0ull;
	}
	for (n = 0; n < 64; n++) {
		board->squares[n] = EMPTY;
	}
	board->bits[EMPTY] = ~board->bits[OCCUPIED];
	board->ply = 0;
	board->enpas = NO_SQUARE;
	board->castle  = C_WK | C_WQ | C_BK | C_BQ;
	
	board->score = 0;
	board->ephash = 0ull;
	board->hash = getHashCode(board);
}

void initBoard(Board* board){
	int p, n;
	
	board->ply = 0;
	board->enpas = NO_SQUARE;
	board->castle = C_WK | C_WQ | C_BK | C_BQ;
	
	board->bits[WP] = R_2;
	board->bits[WN] = R_1 & (R_B | R_G);
	board->bits[WB] = R_1 & (R_C | R_F);
	board->bits[WR] = R_1 & (R_A | R_H);
	board->bits[WQ] = R_1 & R_D;
	board->bits[WK] = R_1 & R_E;
	
	board->bits[BP] = R_7;
	board->bits[BN] = R_8 & (R_B | R_G);
	board->bits[BB] = R_8 & (R_C | R_F);
	board->bits[BR] = R_8 & (R_A | R_H);
	board->bits[BQ] = R_8 & R_D;
	board->bits[BK] = R_8 & R_E;
	
	board->bits[WHITE] = R_1 | R_2;
	board->bits[BLACK] = R_7 | R_8;
	board->bits[OCCUPIED] = (board->bits[WHITE]) | (board->bits[BLACK]);
	board->bits[EMPTY] = ~(board->bits[OCCUPIED]);
	
	for (n = 0; n < 64; n++) {
		if (n/8 == 0 || n/8 == 7) {
			switch (n%8) {
				case 0:
				case 7:
					board->squares[n] = WR;
					break;
				case 1:
				case 6:
					board->squares[n] = WN;
					break;
				case 2:
				case 5:
					board->squares[n] = WB;
					break;
				case 3:
					board->squares[n] = WQ;
					break;
				case 4:
					board->squares[n] = WK;
					break;
			}
		} else if (n/8 == 1 || n/8 == 6) {
			board->squares[n] = WP;
		} else {
			board->squares[n] = EMPTY;
		}
		if (n/8 >= 6) {
			board->squares[n] += BP;
		}
	}
	
	board->score = 0;
	board->ephash = 0ull;
	board->hash = getHashCode(board);
}

//generates a board given Forsyth-Edwards Notation
int genBoard(Board* board, char* fen){
	int i, r, f;
	char *pieces, *side, *castling, *enpassant, *halfmove, *fullmove;
	char str[512];
	Board newBoard; //make our board locally, then copy it over at the end
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
	clearBoard(&newBoard);
	newBoard.ply = 2 * (atol(fullmove) - 1);
	if (newBoard.ply < 0) return 0;
	if (strcmp(side, "b") == 0) {
		newBoard.ply++;
	} else if (strcmp(side, "w") != 0) {
		return 0;
	}
	newBoard.castle = 0;
	if (strcmp(castling, "-") != 0) {
		if (*castling == 'K' || *castling == 'A') {
			newBoard.castle |= C_WK;
			++castling;
		}
		if (*castling == 'Q' || *castling == 'H') {
			newBoard.castle |= C_WQ;
			++castling;
		}
		if (*castling == 'k' || *castling == 'a') {
			newBoard.castle |= C_BK;
			++castling;
		}
		if (*castling == 'q' || *castling == 'h') {
			newBoard.castle |= C_BQ;
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
		newBoard.enpas = 8*rank + file;
	}
	newBoard.ephash = EPHASH(&newBoard);
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
			newBoard.bits[p] |= BIT(r*8 + f);
			newBoard.bits[OCCUPIED] |= BIT(r*8 + f);
			newBoard.bits[WHITE + ISBLACK[p]] |= BIT(r*8 + f);
			newBoard.score += VAL[p];
			newBoard.squares[r*8 + f] = p;
			++f;
		}
	}
	if (r != 0 && f != 8) return 0;
	newBoard.bits[EMPTY] = ~newBoard.bits[OCCUPIED];
	newBoard.hash = getHashCode(&newBoard);
	*board = newBoard;
	return 1;
}

void removePiece(Board* board, int sq){
	int p;
	bitboard b;
	p = board->squares[sq];
	if (p == EMPTY) return;
	
	b = BIT(sq);
	board->bits[p] &= ~b;
	board->bits[COLOR[p]] &= ~b;
	board->bits[OCCUPIED] &= ~b;
	board->bits[EMPTY] |= b;
	board->score -=VAL[p];
	board->squares[sq] = EMPTY;
	board->hash ^= PHASH(p, sq);
}

//only use when you know at compile time that sq is currently empty
void placePiece(Board *board, int sq, int p) {
	bitboard b;
	if (p == EMPTY) return;
	b = BIT(sq);
	board->bits[p] |= b;
	board->bits[COLOR[p]] |= b;
	board->bits[OCCUPIED] |= b;
	board->bits[EMPTY] &= ~b;
	board->score += VAL[p];
	board->squares[sq] = p;
	board->hash ^= PHASH(p, sq);
}

void setPiece(Board *board, int sq, int p) {
	int p0;
	bitboard b;
	p0 = board->squares[sq];
	if (p == p0) return;
	
	b = BIT(sq);
	board->bits[p0] &= ~b;
	board->bits[p] |= b;
	board->bits[COLOR[p0]] &= ~b;
	board->bits[COLOR[p]] |= b;
	board->bits[OCCUPIED] =  ~(board->bits[EMPTY]);
	board->score +=VAL[p] - VAL[p0];
	board->squares[sq] = p;
	if (p0 <= BK) {
		board->hash ^= PHASH(p0, sq);
	}
	if (p <= BK) {
		board->hash ^= PHASH(p, sq);
	}
}

int makeMove(Board* board, int mov) {
	Board orig = *board;
	int i, p, sq0, sq1, sd, prom, cast, enpas = NO_SQUARE;
	sq0 = FROM(mov);
	sq1 = TO(mov);
	sd = 6*(board->ply%2);
	prom = PROM(mov) + sd;
	bitboard b0, b1;
	b0 = BIT(sq0);
	b1 = BIT(sq1);
	p = board->squares[sq0];
	
	//update castling status from changes to the rooks.  changes to the kings are handled later.
	if ((sq0 == A1 || sq1 == A1) && (board->castle & C_WQ)) {
		board->castle &= (~C_WQ);
		board->hash ^= CASTLEHASH(C_WQ);
	} else if ((sq0 == H1 || sq1 == H1) && (board->castle & C_WK)) {
		board->castle &= (~C_WK);
		board->hash ^= CASTLEHASH(C_WK);
	} else if ((sq0 == A8 || sq1 == A8) && (board->castle & C_BQ)) {
		board->castle &= (~C_BQ);
		board->hash ^= CASTLEHASH(C_BQ);
	} else if ((sq0 == H8 || sq1 == H8) && (board->castle & C_BK)) {
		board->castle &= (~C_BK);
		board->hash ^= CASTLEHASH(C_BK);
	}
	
	removePiece(board, sq0);
	setPiece(board, sq1, p);
	
	switch (p) {	//special logic for special pieces
		int mask;
		case WP:
			if (sq1 - sq0 == 16){    //double ahead, set enpassant
				enpas = sq0+8;
			} else if (sq1  == board->enpas){  //enpassant capture
				removePiece(board, board->enpas - 8);
			} else if (sq1/8 == 7){ //pawn promotion
				setPiece(board, sq1, prom);
			}
			break;
		case BP:
			if (sq0 - sq1 == 16){		//double ahead, set enpassant
				enpas = sq1+8;
			} else if (sq1  == board->enpas){		//enpassant capture
				removePiece(board, board->enpas + 8);
			} else if (sq1/8 == 0){	//pawn promotion
				//removePiece(board, sq1);
				setPiece(board, sq1, prom);
			}
			break;
			
		case WK:
		case BK:
			//prevent castling from side who moved their king
			mask = (p == WK) ? (C_WK | C_WQ) : (C_BK | C_BQ);
			int m2 = mask & board->castle;
			int bit = C_WK;
			while (bit <= m2) {		//update our hash value.
				if (m2 & bit) {
					board->hash ^= CASTLEHASH(bit);
				}
				bit <<= 1;
			}
			board->castle &= (~mask);
			if (sq1 - sq0 == 2){	//king side castling
				removePiece(board, sq1+1);
				placePiece(board, sq1-1, WR + sd);
			} else if (sq0 - sq1 == 2){  //queen side castling
				removePiece(board, sq1-2);
				placePiece(board, sq1+1, WR + sd);
			}
			
	}
	board->hash ^= board->ephash;		//unapply the last ep hash;
	board->ply++;										//increment board ply, remember this needs to be updated before calling EPHASH
	board->hash ^= WHITETURNHASH;
	board->enpas = enpas;		//update en passant square
	board->ephash = EPHASH(board);	//calculate/store new ep has
	board->hash ^= board->ephash;   //apply new ep hash
	
	if (inCheck(board, (sd == 0 ? WHITE : BLACK))) {
		unmakeMove(board, mov);
		return 0;
	}
	return 1;
	//board->hash ^= PHASH(p, sq0);
	//board->hash ^= PHASH(p, sq1);
}


void unmakeMove(Board *board, int mov) {
	int sq0, sq1, cap, prom, enpas, capSq, p, cast, sd;
	bitboard diff;
	
	sq0 = FROM(mov);
	sq1 = TO(mov);
	cap = CAP(mov);
	prom = PROM(mov);
	enpas = EP(mov);
	p = board->squares[sq1];
	sd = ISBLACK[p] * 6;
	if (prom != EMPTY) p = WP + sd;
	cast = CAST(mov);
	capSq = sq1;
	if (enpas != NO_SQUARE) {
		if (sq1 == enpas && (p == WP || p == BP)) {
			capSq = (sq0 & 0x38) | (sq1 & 0x07); // same file as sq1, same rank as sq0
		}
	}
	diff = board->castle ^ cast;
	while (diff) {
		board->hash ^= CASTLEHASH(diff);
		popBit(&diff);
	}
	board->castle = cast;
	removePiece(board, sq1);
	placePiece(board, sq0, p);
	placePiece(board, capSq, cap);
	if ((p == WK || p == BK) && (sq0 - sq1 == 2 || sq0 - sq1 == -2)){ //castling: move the rook back
		removePiece(board, (sq0 + sq1) / 2);
		if (sq1 > sq0) { //king side
			placePiece(board, sq1 + 1, p - 2);
		} else { // queen side
			placePiece(board, sq1 - 2, p - 2);
		}
	}
	
	board->hash ^= board->ephash;
	board->ply--;
	board->hash ^= WHITETURNHASH;
	board->enpas = enpas;
	board->ephash = EPHASH(board);
	board->hash ^= board->ephash;
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
checks if side is attacking sq on the board.
*/
int sqAttacked(Board *board, int sq, int side) {
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
	if (attackers & board->bits[WP+aSd]) return 1;
	//knight
	attackers = TRANS(KNIGHTMOVE, FILEDIF(sq,E5), RANKDIF(sq, E5));
	if (attackers & board->bits[WN+aSd]) return 1;
	//bishop and queen diagonal
	attackers = pieceCaptures(board, WB+dSd, sq);
	if (attackers & (board->bits[WB+aSd] | board->bits[WQ+aSd]) ) return 1;
	//rook and queen orthogonal
	attackers = pieceCaptures(board, WR+dSd, sq);
	if (attackers & (board->bits[WR+aSd] | board->bits[WQ+aSd]) ) return 1;
	//king
	attackers = pieceCaptures(board, WK+dSd, sq);
	if (attackers & board->bits[WK+aSd]) return 1;
	return 0;
	
	
	
	#if 0
	int sd, i;
	sd = (side == WHITE) ? 0 : 6;
	
	for (i = 0; i < 6; i++) {
		int n, p, sq0;
		bitboard b;
		p = sd + i;
		b = board->bits[p];
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
					if (diagslide(board->bits[OCCUPIED], sq0, sq)){
						return 1;
					}
					break;
				case WR:
				case BR:
					if (orthslide(board->bits[OCCUPIED], sq0, sq)){
						return 1;
					}
					break;
				case WQ:
				case BQ:
					if (diagslide(board->bits[OCCUPIED], sq0, sq) || orthslide(board->bits[OCCUPIED], sq0, sq)){
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

int inCheck(Board *board, int side) {
	int sd, attacker;
	sd = (side == WHITE ? 0 : 6);
	attacker = (side == WHITE ? BLACK : WHITE);
	return sqAttacked(board, bsf(board->bits[WK+sd]), attacker);
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

int validateBoardState(Board *board) {
	int p, n, squares[64], score = 0;
	bitboard occupied = 0, white = 0, black = 0;
	
	for (n = 0; n < 64; n++) {
		squares[n] = EMPTY;
	}
	
	for (p = WP; p <= BK; p++){
		bitboard pbits;
		pbits = board->bits[p];
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
	
	if (white != board->bits[WHITE] || black != board->bits[BLACK] || occupied != board->bits[OCCUPIED] || ~occupied != board->bits[EMPTY]) {
		printf("Incorrect derived bitboards.\n");
		printf("white:\n");
		printBitboard(board->bits[WHITE]);
		printf("\nblack:\n");
		printBitboard(board->bits[BLACK]);
		printf("\noccupied:\n");
		printBitboard(board->bits[OCCUPIED]);
		printf("\nempty:\n");
		printBitboard(board->bits[EMPTY]);
		return 0;
	}
	
	if (score != board->score) {
		printf("Incorrect score, expected: %d, actual: %d\n", score, board->score);
		return 0;
	}
	
	for (n = 0; n < 64; n++) {
		if (squares[n] != board->squares[n]) {
			printf("Incorrect piece buffer.");
			return 0;
		}
	}
	if (board->ephash != EPHASH(board)) {
		printf("Incorrect en passant hash code, expected: %llx, actual: %llx.\n", EPHASH(board), board->ephash);
		return 0;
	}
	
	if (board->hash != getHashCode(board)) {
		printf("Incorrect hash code, expected: %llx, actual: %llx.\n", getHashCode(board), board->hash);
		return 0;
	}
	
	return 1;
}

int compareBoards(Board *b1, Board *b2) {
	int p, sq;
	for (p = 0; p < 15; ++p) {
		if (b1->bits[p] != b2->bits[p]) {
			printf("Bitboard mismatch for piece %d\n", p);
			printBitboard(b1->bits[p]);
			printf("\n");
			printBitboard(b2->bits[p]);
			printf("\n");
			return 0;
		}
	}
	if (b1->ply != b2->ply) {
		printf("Ply mismatch\n%d\n%d\n", b1->ply, b2->ply);
		return 0;
	}
	
	if (b1->enpas != b2->enpas) {
		printf("En passant square mismatch\n%d\n%d\n", b1->enpas, b2->enpas);
		return 0;
	}
	
	if (b1->castle != b2->castle) {
		printf("Castling rights mismatch\n%x\n%x\n", b1->castle, b2->castle);
		return 0;
	}
  if (b1->score != b2->score) {
		printf("Score mismatch\n%d\n%d\n", b1->score, b2->score);
		return 0;
	}
	for (sq = 0; sq < 64; ++sq) {
		if (b1->squares[sq] != b2->squares[sq]) {
			printf("Square mismatch\n");
			printBoard(b1);
			printBoard(b2);
			return 0;
		}
	}
	if (b1->hash != b2->hash) {
		printf("Hash mismatch\n%x\n%x\n", b1->hash, b2->hash);
		return 0;
	}
	if (b1->ephash != b2->ephash) {
		printf("En passant hash mismatch\n%x\n%x\n", b1->hash, b2->hash);
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
int getGameStatus(Board *board) {
	int moves[MAX_MOVES], nMoves, i;
	int sd;
	int check;
	sd = 6*(board->ply%2);
	check = inCheck(board, (sd == 0 ? WHITE : BLACK));
	nMoves = getMoves(board, moves, 0, 0);
	for (i = 0; i < nMoves; ++i) {
		if (makeMove(board, moves[i])) {
			unmakeMove(board, moves[i]);
			return check * (3 + board->ply%2);
		}
	}
	if (check) {
		return 2 - board->ply%2;
	} else {
		return -1;
	}
	/*
	
	int i, sd, opponent, cast, status = 0;
	opponent = BLACK - (board->ply%2);
	sd = 6*(board->ply%2);
	if (sqAttacked(board, bsf(board->bits[WK + sd]), opponent)) {	//no moves, king in check.  checkmate
		status = 3 +  (board->ply%2);
	}
	cast = board->castle;
	for (i = WP; i <= WK; i++) {
		int p, n, sq, cast;
		bitboard b;
		p = sd + i;
		b = board->bits[p];
		cast = board->castle;
		while ((sq = popBit(&b)) != NO_SQUARE){
			bitboard moves;
			int m, cap;
			moves = pieceMoves(board, p, sq);
			while ((m = popBit(&moves)) != NO_SQUARE) {
				int kingSq, move, attacked;
				cap = board->squares[m];
				if ((p == WP || p == BP) && cap == EMPTY && m%8 != sq%8) { //en passant
					cap = BP - sd;
				}
				if ((p == WP && m/8 == 7) || (p == BP && m/8 == 0)) { //pawn promotion
					//king square can be obtained from the old board, as it won't change from a pawn move
					kingSq = bsf(board->bits[WK + sd]);
					
					move = MOV(sq, m, cap, WN, cast, board->enpas);
					makeMove(board, move);
					attacked = sqAttacked(board, kingSq, opponent);
					unmakeMove(board, move);
					if (!attacked){
						return status;
					}
					
					move = MOV(sq, m, cap, WB, cast, board->enpas);
					makeMove(board, move);
					attacked = sqAttacked(board, kingSq, opponent);
					unmakeMove(board, move);
					if (!attacked){
						return status;
					}
					
					move = MOV(sq, m, cap, WR, cast, board->enpas);
					makeMove(board, move);
					attacked = sqAttacked(board, kingSq, opponent);
					unmakeMove(board, move);
					if (!attacked){
						return status;
					}
					
					move = MOV(sq, m, cap, WQ, cast, board->enpas);
					makeMove(board, move);
					attacked = sqAttacked(board, kingSq, opponent);
					unmakeMove(board, move);
					if (!attacked){
						return status;
					}
					
				} else {
					move = MOV(sq, m, cap, EMPTY, cast, board->enpas);
					makeMove(board, move);
					kingSq = bsf(board->bits[WK + sd]);
					attacked = sqAttacked(board, kingSq, opponent);
					unmakeMove(board, move);
					if (!attacked){
						return status;
					}
				}
			}
		}
	}
	if (status) { //no moves, king in check.  checkmate
		return 2 - (board->ply%2);
	}
	return -1;
	*/
}

int getMobility(Board *board) {
	int p, n = 0;
	for (p = WP; p <= WK; ++p) {
		int sq;
		bitboard b;
		b = board->bits[p];
		while ((sq = popBit(&b)) != NO_SQUARE) {
			n += countBits(pieceMoves(board, p, sq));
		}
	}
	
	for (p = BP; p <= BK; ++p) {
		int sq;
		bitboard b;
		b = board->bits[p];
		while ((sq = popBit(&b)) != NO_SQUARE) {
			n -= countBits(pieceMoves(board, p, sq));
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

void orderMoves(Board *board, int numMoves, int moves[]) {
	int tmp;
	int i;
	int scores[MAX_MOVES];
	int tableMove;
	tableMove = getTableMove(board);
	if (numMoves < 2) return;
	for (i = 0; i < numMoves; ++i) {
		if (moves[i] == tableMove) {
			scores[i] = MAX_VAL;
		} else {
			/*
			if (makeMove(board, moves[i])) {
				scores[i] = -eval(board);
				unmakeMove(board, moves[i]);
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

int eval(Board *board) {
	int sign;
	sign = 1 - 2*(board->ply%2);
	//int pointsPerCenter = 35;
	//return board->score + pointsPerCenter * (countBits(board->bits[WHITE] & CENTER) - countBits(board->bits[BLACK] & CENTER));
	//return sign * (board->score + getMobility(board));
	return sign*board->score;
}

int quiescence(Board *board, int alpha, int beta) {
	int numMoves, i, moves[MAX_MOVES], bestMove = 0;
	int nodeType = UPPER;
	int standingPat;
	#define DELTA 200
	standingPat = eval(board); 
	if (standingPat >= beta) {
		return beta;
	}
	if (standingPat > alpha) {  //null move heuristic
		alpha = standingPat;
	}
	if (standingPat + Q_VAL < alpha) { //futility check
		return alpha;
	}
	if (!board->bits[WK] || !board->bits[BK]) {
		return standingPat;
	}
	getTableBounds(board, &alpha, &beta, 0);
	if (alpha >= beta) return alpha;
	numMoves = getMoves(board, moves, 1, 1);
	orderMoves(board, numMoves, moves);

	for (i = 0; i < numMoves; ++i) {
		int score;
		//delta pruning
		if (ABS(VAL[CAP(moves[i])]) + DELTA < alpha) continue;
		if (makeMove(board, moves[i])) {
			score = -quiescence(board, -beta, -alpha);
			unmakeMove(board, moves[i]);
			if (score >= beta) {
				nodeType = LOWER;
				addToTable(board, beta, 0, nodeType, moves[i]);
				return beta;
			}
			if (score > alpha) {
				alpha = score;
				bestMove = moves[i];
			}
		}
	}
	addToTable(board, beta, 0, nodeType, bestMove);
	return alpha;
}

int moveSearch(Board *board, int depth, int *score) {
	int numMoves, i, moves[MAX_MOVES];
	int bestMove;
	int current_depth;
	interrupt_flag = 0;
	for (current_depth = 0; current_depth < depth; ++current_depth) {
		int alpha = MIN_VAL;
		numMoves = getMoves(board, moves, 1, 0);
		orderMoves(board, numMoves, moves);
		for (i = 0; i < numMoves; ++i) {
			int score;
			if (makeMove(board, moves[i])) {
				score = -alphaBeta(board, MIN_VAL, -alpha, current_depth);
				unmakeMove(board, moves[i]);
				if (score > alpha) {
					alpha = score;
					bestMove = moves[i];
				}
			}
		}
	}
	return bestMove;
}

int alphaBeta(Board *board, int alpha, int beta, int depthleft) {
	int numMoves, i, moves[MAX_MOVES];
	int nodeType = UPPER;
	int bestMove = 0;
	int legalMove = 0;
	getTableBounds(board, &alpha, &beta, depthleft);
	if (interrupt_flag) return alpha;
	if (alpha >= beta) return alpha;
	if (depthleft <= 0 || !board->bits[WK] || !board->bits[BK]) {
		return quiescence(board, alpha, beta);
	}
	numMoves = getMoves(board, moves, 1, 0);
	orderMoves(board, numMoves, moves);
	for (i = 0; i < numMoves; ++i) {
		int score;
		if (makeMove(board, moves[i])) {
			legalMove = 1;
			score = -alphaBeta(board, -beta, -alpha, depthleft - 1);
			unmakeMove(board, moves[i]);
			if (score >= beta) {
				nodeType = LOWER;
				addToTable(board, beta, depthleft, nodeType, moves[i]);
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
		if (inCheck(board, (board->ply%2 == 0 ? WHITE : BLACK))) return alpha;
		else return 0;
	}
	addToTable(board, alpha, depthleft, nodeType, bestMove);
	return alpha;
}


int addToTable(Board *board, int score, int depth, int nodeType, int bestMove) {
	tableEntry *t;
	int i;
	i = HASHKEY(board->hash);
	t = &transpositionTable[i];
	if (t->hash == board->hash) { //previously seen position
		if (!(bestMove && ! t->move) && t->depth >= depth) return 0;
	} else if (HASHKEY(t->hash) == i) { //type-2 error
			if (0) return 0; //todo -- add replacement policy
	}
	t->hash = board->hash;
	t->depth = depth;
	t->value = score;
	t->nodeType = nodeType;
	t->move = bestMove;
	t->utility = 0;
	return 1;
}

int getBookMove(Board *board) {
	tableEntry *t;
	int i;
	i = HASHKEY(board->hash);
	t = &transpositionTable[i];
	if (t->hash == board->hash && t->bookMove) {
		//the opening book doesn't contain information like en-passant
		//and castling rights, so we must provide it
		int move;
		int p, sq0, sq1, cap, prom;
		int sd;
		sd = 6 * (board->ply % 2);
		move = t->bookMove;
		sq0 = FROM(move);
		sq1 = TO(move);
		p = board->squares[sq0];
		cap = board->squares[sq1];
		if ((p == WP || p == BP) && cap == EMPTY && sq1%8 != sq0%8) { //en passant
			cap = BP - sd;
		}
		prom = PROM(move);
		return MOV(sq0, sq1, cap, prom, board->castle, board->enpas);
	}
	return 0;
}

int getTableMove(Board *board) {
	tableEntry *t;
	int i;
	i = HASHKEY(board->hash);
	t = &transpositionTable[i];
	if (t->hash == board->hash && t->nodeType != BOOK) {
		++t->utility;
		return t->move;
	} else {
		--t->utility;
		return 0;
	}
}

void getTableBounds(Board *board, int *alpha, int *beta, int depth) {
	tableEntry *t;
	int i;
	i = HASHKEY(board->hash);
	t = &transpositionTable[i];
	if (t->hash == board->hash) {
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


