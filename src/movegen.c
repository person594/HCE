#include "movegen.h"

/*
	warning:  only generates pseudo-legal moves.  some might be non legal
*/
bitboard pieceMoves(Board *board, int p, int sq) {
	int player, i;
	bitboard b0, b1, b2, occupied, empty, friendly, enemy, moves = 0ull;
	b0 = BIT(sq);
	player = ISBLACK[p];
	occupied = board->bits[OCCUPIED];
	empty = board->bits[EMPTY];
	friendly = board->bits[WHITE + player];
	enemy = board->bits[BLACK - player];
	switch (p){
		case BP:
			enemy |= BIT(board->enpas);
			moves = (b0>>8) & empty;
			if (moves && sq/8 == 6){
				moves |= (b0>>16 & empty);
			}
			moves |= (b0>>7 | b0>>9) & (enemy ) & RANKOF(sq-8);
			return moves;
		case WP:
			enemy |= BIT(board->enpas);
			moves = (b0<<8) & empty;
			if (moves && sq/8 == 1){
				moves |= (b0<<16 & empty);
			}
			moves |= (b0<<7 | b0<<9) & (enemy ) & RANKOF(sq+8);
			return moves;
		case WN:
		case BN:
			moves = TRANS(KNIGHTMOVE, FILEDIF(sq,E5), RANKDIF(sq, E5)) & ~friendly;
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
			b2 = RANKOF(sq);
			b1 = b2 & ~((b0<<1)-1);
			i = sf(b1&occupied);
			moves |= (BIT(i+1) - 1) & b1;
			//west
			b1 = b2 & (b0-1);
			i = sr(b1&occupied);
			moves |= ~(BIT(i) - 1) & b1;
			//north
			b2 = FILEOF(sq);
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
			moves = TRANS(KINGMOVE, FILEDIF(sq,E5), RANKDIF(sq, E5)) & ~friendly;
			if (p == WK && !sqAttacked(board, E1, BLACK)){
				if ((board->castle & C_WK) && !sqAttacked(board, F1, BLACK) && !(occupied & 0x60ull)){
					moves |= BIT(G1);
				}
				if ((board->castle & C_WQ) && !sqAttacked(board, D1, BLACK) && !(occupied & 0x0eull)){
					moves |= BIT(C1);
				}
			} else if (p == BK && !sqAttacked(board, E8, WHITE)){
				if ((board->castle & C_BK) && !sqAttacked(board, F8, WHITE) && !(occupied & 0x6000000000000000ull)){
					moves |= BIT(G8);
				}
				if ((board->castle & C_BQ) && !sqAttacked(board, D8, WHITE) && !(occupied & 0x0e00000000000000ull)){
					moves |= BIT(C8);
				}
			}
			
	}
	
	
	return moves;
}


/*
	warning:  only generates pseudo-legal moves.  some might be non legal
*/
bitboard pieceCaptures(Board *board, int p, int sq) {
	int player, i;
	bitboard b0, b1, b2, occupied, empty, friendly, enemy, moves = 0ull;
	b0 = BIT(sq);
	player = ISBLACK[p];
	occupied = board->bits[OCCUPIED];
	empty = board->bits[EMPTY];
	friendly = board->bits[WHITE + player];
	enemy = board->bits[BLACK - player];
	switch (p){
		case BP:
			enemy |= BIT(board->enpas);
			moves = (b0>>7 | b0>>9) & (enemy ) & RANKOF(sq-8);
			return moves;
		case WP:
			enemy |= BIT(board->enpas);
			moves = (b0<<7 | b0<<9) & (enemy ) & RANKOF(sq+8);
			return moves;
		case WN:
		case BN:
			moves = TRANS(KNIGHTMOVE, FILEDIF(sq,E5), RANKDIF(sq, E5)) & enemy;
			break;
		case WQ:
		case BQ:
		
		case WB:
		case BB:
			//northeast
			b2 = PDIAG(sq);
			b1 = b2 & ~((b0<<1)-1);
			i = sf(b1&occupied);
			moves |= (BIT(i+1) - 1) & b1;
			//southwest
			b1 = b2 & (b0-1);
			i = sr(b1&occupied);
			moves |= ~(BIT(i)-1) & b1;
			//southeast
			b2 = NDIAG(sq);
			b1 = b2 & (b0-1);
			i = sr(b1&occupied);
			moves |= ~(BIT(i)-1) & b1;
			//northwest
			b1 = b2 & ~((b0<<1)-1);
			i = sf(b1&occupied);
			moves |= (BIT(i+1) - 1) & b1;
			moves &= enemy;
			if (p == WB | p == BB){
				break;
			}
		case WR:
		case BR:
			//east
			b2 = RANKOF(sq);
			b1 = b2 & ~((b0<<1)-1);
			i = sf(b1&occupied);
			moves |= (BIT(i+1) - 1) & b1;
			//west
			b1 = b2 & (b0-1);
			i = sr(b1&occupied);
			moves |= ~(BIT(i) - 1) & b1;
			//north
			b2 = FILEOF(sq);
			b1 = b2 & ~((b0<<1)-1);
			i = sf(b1&occupied);
			moves |= (BIT(i+1) - 1) & b1;	
			//south
			b1 = b2 & (b0-1);
			i = sr(b1&occupied);
			moves |= ~(BIT(i) - 1) & b1;
			moves &= enemy;
			break;
		case WK:
		case BK:
			moves = TRANS(KINGMOVE, FILEDIF(sq,E5), RANKDIF(sq, E5)) & enemy;
	}
	
	
	return moves;
}


//finds moves and puts them in moves.  make sure moves is big enough lol.  returns number of moves found.
int getMoves(Board *board, int* moves, int useBook, int onlyCaptures) {
	int i, n, sd, count = 0, cast, enpas, bookMove, fromBook = 0, *moves0;
	moves0 = moves;
	sd = 6*(board->ply%2);
	
	//both of the next two lookups can generate non-captures, and possibly
	//invalid moves in the case of a hash collision.  We go throught the
	//full list of moves anyway, and ensure the remembered move is present.
	
	bookMove = useBook ? getBookMove(board) : 0;
	cast = board->castle;
	for (i = WP; i <= WK; ++i) {
		int p, sq;
		bitboard b;
		p = sd + i;
		b = board->bits[p];
		while ((sq = popBit(&b)) != NO_SQUARE) {
			int m, cap;
			bitboard movebits;
			if (onlyCaptures){
				movebits = pieceCaptures(board, p, sq);
			} else {
				movebits = pieceMoves(board, p, sq);
			}
			while ((m = popBit(&movebits)) != NO_SQUARE) {
				int move;
				cap = board->squares[m];
				if ((p == WP || p == BP) && cap == EMPTY && sq%8 != m%8) { //en passant
					cap = BP - sd;
				}
				if ((p == WP && m/8 == 7) || (p == BP && m/8 == 0)) {	//pawn promotion
					move = MOV(sq, m, cap, WQ, cast, board->enpas);
					if (move == bookMove) {
							*moves0 = move;
							return 1;
					} else {
						*moves++ = move;
					}
					move = MOV(sq, m, cap, WN, cast, board->enpas);
					if (move == bookMove) {
							*moves0 = move;
							return 1;
					} else {
						*moves++ = move;
					}
					move = MOV(sq, m, cap, WR, cast, board->enpas);
					if (move == bookMove) {
							*moves0 = move;
							return 1;
					} else {
						*moves++ = move;
					}
					move = MOV(sq, m, cap, WB, cast, board->enpas);
					if (move == bookMove) {
							*moves0 = move;
							return 1;
					} else {
						*moves++ = move;
					}
				} else {
					move = MOV(sq, m, cap, EMPTY, cast, board->enpas);
					if (move == bookMove) {
						*moves0 = move;
						return 1;
					} else {
						*moves++ = move;
					}
				}
			}
		}
	}
	return moves - moves0;
}
