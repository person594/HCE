#include <stdio.h>

#include "defs.h"


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

void printBoard(Board board){
	char chars[64];
	int p, n, r, c;
	for (n = 0; n < 64; n++){
		chars[n] = ' ';
	}
	for (p = 0; p < 12; p++) {
		int pos;
		bitboard b = board.bits[p];
		while ((pos = popBit(&b)) != NO_SQUARE) {
			chars[pos] = getSymbol(p);
		}
	}
	/* en passant square
	if (board.enpas != NO_SQUARE) {
		chars[board.enpas] = 'x';
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
	//printf("%llx\n", board.hash);
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
	board->hash = getHashCode(*board);
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
	board->hash = getHashCode(*board);
}

//generates a board given Forsyth-Edwards Notation
genBoard(Board* board, char* str, int active, int castle, int enpas, int hmclock, int movenum){
	int i, r = 7, c = 0;
	clearBoard(board);
	board->ply = 2*(movenum-1) + (active == BLACK);
	board->enpas = enpas;
	board->ephash = EPHASH(*board);
	board->castle = castle;
	for (i = 0; str[i]; i++, c++) {
		int p;
		char ch;
		ch = str[i];
		if (ch == '/'){
			r--;
			c = -1;
			continue;
		}
		if (ch > '0' && ch <= '9'){
			c += (ch - '1');
			continue;
		}
		p = getPiece(str[i]);
		board->bits[p] |= BIT(r*8 + c);
		board->bits[OCCUPIED] |= BIT(r*8 + c);
		board->bits[WHITE + ISBLACK[p]] |= BIT(r*8 + c);
		board->score += VAL[p];
		board->squares[r*8 + c] = p;
	}
	board->bits[EMPTY] = ~board->bits[OCCUPIED];
	board->hash = getHashCode(*board);
}

void clearSq(Board* board, int sq){
	int p;
	bitboard b;
	b = BIT(sq);
	p = board->squares[sq];
	
	board->bits[p] &= ~b;
	board->bits[COLOR[p]] &= ~b;
	board->bits[OCCUPIED] &= ~b;
	board->bits[EMPTY] |= b;
	board->score -=VAL[p];
	board->squares[sq] = EMPTY;
	if (p <= BK) {
		board->hash ^= PHASH(p, sq);
	}
}

void makeMove(Board* board, move mov) {
	Board orig = *board;
	int i, p, sq0, sq1, prom, enpas = NO_SQUARE;
	sq0 = FROM(mov);
	sq1 = TO(mov);
	prom = PROM(mov) + 6*(board->ply%2);
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
	
	clearSq(board, sq1);					//empty the destination square
	
	//move the piece, updating all aplicable bitboards.
	board->bits[p] &= ~b0;
	board->bits[COLOR[p]] &= ~b0;
	board->bits[OCCUPIED] &= ~b0;
	board->bits[EMPTY] |= b0;
	board->squares[sq0] = EMPTY;
	
	board->bits[p] |= b1;
	board->bits[COLOR[p]] |= b1;
	board->bits[OCCUPIED] |= b1;
	board->bits[EMPTY] &= ~b1;
	board->squares[sq1] = p; 
	
	switch (p) {	//special logic for special pieces
		int mask;
		case WP:
			if (sq1 - sq0 == 16){		//double ahead, set enpassant
				enpas = sq0+8;
			} else if (sq1  == board->enpas){		//enpassant capture
				clearSq(board, board->enpas - 8);
			} else if (sq1/8 == 7){	//pawn promotion
				//clearSq(board, sq1);
				board->bits[p] &= ~b1;
				board->bits[prom] |= b1;
				board->score += VAL[prom] - VAL[WP];
				board->squares[sq1] = prom;
				board->hash ^= PHASH(p, sq1) ^ PHASH(prom, sq1);
			}
			break;
		case BP:
			if (sq0 - sq1 == 16){		//double ahead, set enpassant
				enpas = sq1+8;
			} else if (sq1  == board->enpas){		//enpassant capture
				clearSq(board, board->enpas + 8);
			} else if (sq1/8 == 0){	//pawn promotion
				//clearSq(board, sq1);
				board->bits[p] &= ~b1;
				board->bits[prom] |= b1;
				board->score += VAL[prom] - VAL[BP];
				board->squares[sq1] = prom;
				board->hash ^= PHASH(p, sq1) ^ PHASH(prom, sq1);
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
				makeMove(board, MOV(sq1+1, sq1-1, 0));
				board->ply--;
				board->hash ^= WHITETURNHASH;
			} else if (sq0 - sq1 == 2){  //queen side castling
				makeMove(board, MOV(sq1-2, sq1+1, 0));
				board->ply--;
				board->hash ^= WHITETURNHASH;
			}
			
	}
	board->hash ^= board->ephash;		//unapply the last ep hash;
	board->ply++;										//increment board ply, remember this needs to be updated before calling EPHASH
	board->hash ^= WHITETURNHASH;
	board->enpas = enpas;		//update en passant square
	board->ephash = EPHASH(*board);	//calculate/store new ep has
	board->hash ^= board->ephash;   //apply new ep hash
	
	board->hash ^= PHASH(p, sq0);
	board->hash ^= PHASH(p, sq1);
}

int diagslide(bitboard occupied, int p0, int p1) {
	int delta, p;
	//quickly exit if the two positions are not diagonal
	if (ABS(ROWDIF(p0, p1)) != ABS(RANKDIF(p0, p1))){
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
	if (SAMERANK(p0, p1) == SAMEROW(p0, p1)){	//ensure p0 and p1 share either a row or rank, not both or neither
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
int sqAttacked(Board board, int sq, int side) {
	int sd, i;
	sd = (side == WHITE) ? 0 : 6;
	
	for (i = 0; i < 6; i++) {
		int n, p, sq0;
		bitboard b;
		p = sd + i;
		b = board.bits[p];
		while ((sq0 = popBit(&b)) != NO_SQUARE) {
			/*
			switch over each piece type and check if it is threatening the given square.
			we need not check the piece's color, as we are only looping over p values of the proper color.
			*/
			switch (p) {
				case WP:
					if (side == WHITE && RANKDIF(sq, sq0) == 1 && (ROWDIF(sq, sq0) == 1 || ROWDIF(sq, sq0) == -1)){
						return 1;
					}
					break;
				case BP:
					if (side == BLACK && RANKDIF(sq, sq0) == -1 && (ROWDIF(sq, sq0) == 1 || ROWDIF(sq, sq0) == -1)){
						return 1;
					}
					break;
				case WN:
				case BN:
					if ((ABS(RANKDIF(sq, sq0)) == 2 && ABS(ROWDIF(sq, sq0)) == 1) || (ABS(RANKDIF(sq, sq0)) == 1 && ABS(ROWDIF(sq, sq0)) == 2)){
						return 1;
					}
					break;
				case WB:
				case BB:
					if (diagslide(board.bits[OCCUPIED], sq0, sq)){
						return 1;
					}
					break;
				case WR:
				case BR:
					if (orthslide(board.bits[OCCUPIED], sq0, sq)){
						return 1;
					}
					break;
				case WQ:
				case BQ:
					if (diagslide(board.bits[OCCUPIED], sq0, sq) || orthslide(board.bits[OCCUPIED], sq0, sq)){
						return 1;
					}
					break;
				case WK:
				case BK:
					if (ABS(RANKDIF(sq0, sq)) <= 1 && ABS(ROWDIF(sq0, sq)) <= 1 && sq0 != sq){
						return 1;
					}
			}
		}
	}
	return 0;
}

/*
	warning:  only generates pseudo-legal moves.  some might be non legal
*/
bitboard pieceMoves(Board board, int p, int sq) {
	int player, i;
	bitboard b0, b1, b2, occupied, empty, friendly, enemy, moves = 0ull;
	b0 = BIT(sq);
	player = ISBLACK[p];
	occupied = board.bits[OCCUPIED];
	empty = board.bits[EMPTY];
	friendly = board.bits[WHITE + player];
	enemy = board.bits[BLACK - player];
	switch (p){
		case BP:
			enemy |= BIT(board.enpas);
			moves = (b0>>8) & empty;
			if (moves && sq/8 == 6){
				moves |= (b0>>16 & empty);
			}
			moves |= (b0>>7 | b0>>9) & (enemy ) & RANK(sq-8);
			return moves;
		case WP:
			enemy |= BIT(board.enpas);
			moves = (b0<<8) & empty;
			if (moves && sq/8 == 1){
				moves |= (b0<<16 & empty);
			}
			moves |= (b0<<7 | b0<<9) & (enemy ) & RANK(sq+8);
			return moves;
		case WN:
		case BN:
			moves = TRANS(KNIGHTMOVE, ROWDIF(sq,E5), RANKDIF(sq, E5)) & ~friendly;
			break;
		case WQ:
		case BQ:
		
		case WB:
		case BB:
			//northeast
			b2 = PDIAG(sq);
			b1 = b2 & ~((b0<<1)-1);
			i = sf(b1&occupied);
			//moves |= (BIT(i+9) - b0) & b2;
			moves |= (BIT(i+1) - 1) & b1;
			//southwest
			b1 = b2 & (b0-1);
			i = sr(b1&occupied);
			//moves |= (b0 - BIT(i)) & b2;
			moves |= ~(BIT(i)-1) & b1;
			//southeast
			b2 = NDIAG(sq);
			b1 = b2 & (b0-1);
			i = sr(b1&occupied);
			//moves |= (b0 - BIT(i)) & b2;
			moves |= ~(BIT(i)-1) & b1;
			//northwest
			b1 = b2 & ~((b0<<1)-1);
			i = sf(b1&occupied);
			//moves |= (BIT(i+7) - b0) & b2;
			moves |= (BIT(i+1) - 1) & b1;
			moves &= ~friendly;
			if (p == WB | p == BB){
				break;
			}
		case WR:
		case BR:
			//east
			b2 = RANK(sq);
			b1 = b2 & ~((b0<<1)-1);
			i = sf(b1&occupied);
			moves |= (BIT(i+1) - 1) & b1;
			//west
			b1 = b2 & (b0-1);
			i = sr(b1&occupied);
			moves |= ~(BIT(i) - 1) & b1;
			//north
			b2 = ROW(sq);
			b1 = b2 & ~((b0<<1)-1);
			i = sf(b1&occupied);
			moves |= (BIT(i+1) - 1) & b1;	
			//south
			b1 = b2 & (b0-1);
			i = sr(b1&occupied);
			moves |= ~(BIT(i) - 1) & b1;
			moves &= ~friendly;
			break;
		case WK:
		case BK:
			moves = TRANS(KINGMOVE, ROWDIF(sq,E5), RANKDIF(sq, E5)) & ~friendly;
			if (p == WK && !sqAttacked(board, E1, BLACK)){
				if ((board.castle & C_WK) && !sqAttacked(board, F1, BLACK) && !(occupied & 0x60ull)){
					moves |= BIT(G1);
				}
				if ((board.castle & C_WQ) && !sqAttacked(board, D1, BLACK) && !(occupied & 0x0eull)){
					moves |= BIT(C1);
				}
			} else if (p == BK && !sqAttacked(board, E8, WHITE)){
				if ((board.castle & C_BK) && !sqAttacked(board, F8, WHITE) && !(occupied & 0x6000000000000000ull)){
					moves |= BIT(G8);
				}
				if ((board.castle & C_BQ) && !sqAttacked(board, D8, WHITE) && !(occupied & 0x0e00000000000000ull)){
					moves |= BIT(C8);
				}
			}
			
	}
	
	
	return moves;
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

int validateBoardState(Board board) {
	int p, n, squares[64], score = 0;
	bitboard occupied = 0, white = 0, black = 0;
	
	for (n = 0; n < 64; n++) {
		squares[n] = EMPTY;
	}
	
	for (p = WP; p <= BK; p++){
		if (occupied & board.bits[p]) {
			printf("Multiple pieces at a single square.\n");
			return 0;
		}
		occupied |= board.bits[p];
		if (p < BP) {
			white |= board.bits[p];
		} else {
			black |= board.bits[p];
		}
		score += VAL[p] * countBits(board.bits[p]);
		while (board.bits[p]) {
			squares[popBit(&board.bits[p])] = p;
		}
		
	}
	//generate the rest of our bitboards:
	
	if (white != board.bits[WHITE] || black != board.bits[BLACK] || occupied != board.bits[OCCUPIED] || ~occupied != board.bits[EMPTY]) {
		printf("Incorrect derived bitboards.\n");
		printf("white:\n");
		printBitboard(board.bits[WHITE]);
		printf("\nblack:\n");
		printBitboard(board.bits[BLACK]);
		printf("\noccupied:\n");
		printBitboard(board.bits[OCCUPIED]);
		printf("\nempty:\n");
		printBitboard(board.bits[EMPTY]);
		return 0;
	}
	
	if (score != board.score) {
		printf("Incorrect score, expected: %d, actual: %d\n", score, board.score);
		return 0;
	}
	
	for (n = 0; n < 64; n++) {
		if (squares[n] != board.squares[n]) {
			printf("Incorrect piece buffer.");
			return 0;
		}
	}
	if (board.ephash != EPHASH(board)) {
		printf("Incorrect en passant hash code, expected: %llx, actual: %llx.\n", EPHASH(board), board.ephash);
		return 0;
	}
	
	if (board.hash != getHashCode(board)) {
		printf("Incorrect hash code, expected: %llx, actual: %llx.\n", getHashCode(board), board.hash);
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
int getGameStatus(Board board) {
	int i, sd, opponent, status = 0;
	opponent = BLACK - (board.ply%2);
	sd = 6*(board.ply%2);
	if (sqAttacked(board, bsf(board.bits[WK + sd]), opponent)) {	//no moves, king in check.  checkmate
		status = 3 +  (board.ply%2);
	}
	for (i = WP; i <= WK; i++) {
		int p, n, sq;
		bitboard b;
		p = sd + i;
		b = board.bits[p];
		while ((sq = popBit(&b)) != NO_SQUARE){
			bitboard moves;
			int m, move;
			moves = pieceMoves(board, p, sq);
			while ((m = popBit(&moves)) != NO_SQUARE) {
				Board b2;
				int kingSq;
				b2 = board;
				if ((p == WP && m/8 == 7) || (p == BP && m/8 == 0)) { //pawn promotion
					//king square can be obtained from the old board, as it won't change from a pawn move
					kingSq = bsf(board.bits[WK + sd]);
					makeMove(&b2, MOV(sq, m, WN));
					if (!sqAttacked(b2, kingSq, opponent)){
						return status;
					}
					
					b2 = board;
					makeMove(&b2, MOV(sq, m, WB));
					if (!sqAttacked(b2, kingSq, opponent)){
						return status;
					}
					
					b2 = board;
					makeMove(&b2, MOV(sq, m, WR));
					if (!sqAttacked(b2, kingSq, opponent)){
						return status;
					}
				}
				b2 = board;
				makeMove(&b2, MOV(sq, m, WQ));
				kingSq = bsf(b2.bits[WK + sd]);
				if (!sqAttacked(b2, kingSq, opponent)){
					return status;
				}
			}
		}
	}
	if (status) {	//no moves, king in check.  checkmate
		return 2 - (board.ply%2);
	}
	return -1;
}

//finds moves and puts them in moves.  make sure moves is big enough lol.  returns number of moves found.
int getMoves(Board board, int* moves) {
	int i, n, sd, count = 0;
	sd = 6*(board.ply%2);
	if (HASENTRY(board.hash)) {	//if we found a previous good response to this position, try it first.  note it will be in the list twice. yolo.
		*moves = transpositionTable[HASHKEY(board.hash)].move;
		if (*moves > 0) {
			moves++;
			count++;
		}
	}
	for (i = WK; i >= WP; i--) {
		int p, sq;
		bitboard b;
		p = sd + i;
		b = board.bits[p];
		while ((sq = popBit(&b)) != NO_SQUARE) {
			int m;
			bitboard movebits = pieceMoves(board, p, sq);
			while ((m = popBit(&movebits)) != NO_SQUARE) {
				if ((p == WP && m/8 == 7) || (p == BP && m/8 == 0)) {	//pawn promotion
					*moves++ = MOV(sq, m, WN);
					*moves++ = MOV(sq, m, WB);
					*moves++ = MOV(sq, m, WR);
					count += 3;
				}
				*moves++ = MOV(sq, m, WN);
				count++;
			}
		}
	}
	return count;
}

int moveSearch(Board board, int depth, int* score) {
	int numMoves, moves[120], sign, move = -1, i, nextMove;
	numMoves = getMoves(board, moves);
	if (board.ply % 2) {	//black to move, minimize
		int min = VAL[WK];
		for (i = 0; i < numMoves; i++) {
			Board b2;
			int s;
			b2 = board;
			makeMove(&b2, moves[i]);
			s = alphaBetaMax(b2, VAL[BK], min, depth, &nextMove);
			if (s < min) {
				min = s;
				move = moves[i];
			}
		}
	} else {							//white to move, maximize
		int max = VAL[BK];
		for (i = 0; i < numMoves; i++) {
			Board b2;
			int s;
			b2 = board;
			makeMove(&b2, moves[i]);
			s = alphaBetaMin(b2, max, VAL[WK], depth, &nextMove);
			if (s > max) {
				max = s;
				move = moves[i];
			}
		}
	}
	
	return move;
}

inline int eval(Board board) {
	int pointsPerCenter = 35;
	return board.score + pointsPerCenter * (countBits(board.bits[WHITE] & CENTER) - countBits(board.bits[BLACK] & CENTER));
}

//alpha = lower bound, beta = upper bound
int alphaBetaMax(Board board, int alpha, int beta, int depthleft, int* nextMove) {
	int numMoves, moves[120], i;
	*nextMove = -1;
	if (depthleft <= 0 || !board.bits[WK] || !board.bits[BK]) {
		return eval(board);
	}
	numMoves = getMoves(board, moves);
	for (i = 0; i < numMoves; i++) {
		int score;
		Board b2;
		b2 = board;
		makeMove(&b2, moves[i]);
		
		tableEntry* t = &transpositionTable[HASHKEY(b2.hash)];
		if (HASENTRY(b2.hash)) {
			if (t->depth >= depthleft) {
				//printf("yay\n");
				score = t->value;
			} else {
				score = alphaBetaMin(b2, alpha, beta, depthleft - 1, &t->move);
				t->value = score;
				t->depth = depthleft;
			}
		} else {
			score = alphaBetaMin(b2, alpha, beta, depthleft - 1, &t->move);
			t->hash = b2.hash;
			t->value = score;
			t->depth = depthleft;
		}
		
		if( score >= beta ) {
			*nextMove = moves[i];
			return beta;   // fail hard beta-cutoff
		} if( score > alpha ) {
			*nextMove = moves[i];
			alpha = score; // alpha acts like max in MiniMax
		}
	}
	return alpha;
}

int alphaBetaMin(Board board, int alpha, int beta, int depthleft, int* nextMove) {
	int numMoves, moves[120], i;
	*nextMove = -1;
	if (depthleft <= 0 || !board.bits[WK] || !board.bits[BK]) {
		return eval(board);
	}
	numMoves = getMoves(board, moves);
	for (i = 0; i < numMoves; i++) {
		int score;
		Board b2;
		b2 = board;
		makeMove(&b2, moves[i]);
		tableEntry* t = &transpositionTable[HASHKEY(b2.hash)];
		if (HASENTRY(b2.hash)) {
			if (t->depth >= depthleft) {
				//printf("yay\n");
				score = t->value;
			} else {
				score = alphaBetaMax(b2, alpha, beta, depthleft - 1, &t->move);
				t->value = score;
				t->depth = depthleft;
			}
		} else {
			score = alphaBetaMax(b2, alpha, beta, depthleft - 1, &t->move);
			t->hash = b2.hash;
			t->value = score;
			t->depth = depthleft;
		}
		//score = alphaBetaMax(b2, alpha, beta, depthleft - 1 );
		if (score <= alpha) {
			*nextMove = moves[i];
			return alpha;   // fail hard beta-cutoff
		} if( score < beta ) {
			*nextMove = moves[i];
			beta = score; // alpha acts like max in MiniMax
		}
	}
	return beta;
}

