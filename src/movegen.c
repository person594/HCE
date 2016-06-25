#include "movegen.h"
#include "move.h"
#include "transpositiontable.h"

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
	warning:  only generates pseudo-legal moves.  some might be non legal
*/
bitboard pieceMoves(Position *pos, int p, int sq) {
	int player, i;
	bitboard b0, b1, b2, occupied, empty, friendly, enemy, moves = 0ull;
	b0 = BIT(sq);
	player = ISBLACK[p];
	occupied = pos->bits[OCCUPIED];
	empty = pos->bits[EMPTY];
	friendly = pos->bits[WHITE + player];
	enemy = pos->bits[BLACK - player];
	switch (p){
		case BP:
			enemy |= BIT(pos->enpas);
			moves = (b0>>8) & empty;
			if (moves && sq/8 == 6){
				moves |= (b0>>16 & empty);
			}
			moves |= (b0>>7 | b0>>9) & (enemy ) & RANKOF(sq-8);
			return moves;
		case WP:
			enemy |= BIT(pos->enpas);
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
			if (p == WK && !sqAttacked(pos, E1, BLACK)){
				if ((pos->castle & C_WK) && !sqAttacked(pos, F1, BLACK) && !(occupied & 0x60ull)){
					moves |= BIT(G1);
				}
				if ((pos->castle & C_WQ) && !sqAttacked(pos, D1, BLACK) && !(occupied & 0x0eull)){
					moves |= BIT(C1);
				}
			} else if (p == BK && !sqAttacked(pos, E8, WHITE)){
				if ((pos->castle & C_BK) && !sqAttacked(pos, F8, WHITE) && !(occupied & 0x6000000000000000ull)){
					moves |= BIT(G8);
				}
				if ((pos->castle & C_BQ) && !sqAttacked(pos, D8, WHITE) && !(occupied & 0x0e00000000000000ull)){
					moves |= BIT(C8);
				}
			}
			
	}
	
	
	return moves;
}


/*
	warning:  only generates pseudo-legal moves.  some might be non legal
*/
bitboard pieceCaptures(Position *pos, int p, int sq) {
	int player, i;
	bitboard b0, b1, b2, occupied, empty, friendly, enemy, moves = 0ull;
	b0 = BIT(sq);
	player = ISBLACK[p];
	occupied = pos->bits[OCCUPIED];
	empty = pos->bits[EMPTY];
	friendly = pos->bits[WHITE + player];
	enemy = pos->bits[BLACK - player];
	switch (p){
		case BP:
			enemy |= BIT(pos->enpas);
			moves = (b0>>7 | b0>>9) & (enemy ) & RANKOF(sq-8);
			return moves;
		case WP:
			enemy |= BIT(pos->enpas);
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
}

int inCheck(Position *pos, int side) {
	int sd, attacker;
	sd = (side == WHITE ? 0 : 6);
	attacker = (side == WHITE ? BLACK : WHITE);
	return sqAttacked(pos, bsf(pos->bits[WK+sd]), attacker);
}



//finds moves and puts them in moves.  make sure moves is big enough lol.  returns number of moves found.
int getMoves(Position *pos, int* moves, int useBook, int onlyCaptures) {
	int i, n, sd, count = 0, cast, enpas, bookMove, fromBook = 0, *moves0;
	moves0 = moves;
	sd = 6*(pos->ply%2);
	
	//both of the next two lookups can generate non-captures, and possibly
	//invalid moves in the case of a hash collision.  We go throught the
	//full list of moves anyway, and ensure the remembered move is present.
	
	bookMove = useBook ? getBookMove(pos) : 0;
	cast = pos->castle;
	for (i = WP; i <= WK; ++i) {
		int p, sq;
		bitboard b;
		p = sd + i;
		b = pos->bits[p];
		while ((sq = popBit(&b)) != NO_SQUARE) {
			int m, cap;
			bitboard movebits;
			if (onlyCaptures){
				movebits = pieceCaptures(pos, p, sq);
			} else {
				movebits = pieceMoves(pos, p, sq);
			}
			while ((m = popBit(&movebits)) != NO_SQUARE) {
				int move;
				cap = pos->squares[m];
				if ((p == WP || p == BP) && cap == EMPTY && sq%8 != m%8) { //en passant
					cap = BP - sd;
				}
				if ((p == WP && m/8 == 7) || (p == BP && m/8 == 0)) {	//pawn promotion
					move = MOV(sq, m, cap, WQ, cast, pos->enpas);
					if (move == bookMove) {
							*moves0 = move;
							return 1;
					} else {
						*moves++ = move;
					}
					move = MOV(sq, m, cap, WN, cast, pos->enpas);
					if (move == bookMove) {
							*moves0 = move;
							return 1;
					} else {
						*moves++ = move;
					}
					move = MOV(sq, m, cap, WR, cast, pos->enpas);
					if (move == bookMove) {
							*moves0 = move;
							return 1;
					} else {
						*moves++ = move;
					}
					move = MOV(sq, m, cap, WB, cast, pos->enpas);
					if (move == bookMove) {
							*moves0 = move;
							return 1;
					} else {
						*moves++ = move;
					}
				} else {
					move = MOV(sq, m, cap, EMPTY, cast, pos->enpas);
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


int perftTest(Position *pos, int depth){
	int i, nMoves, sum = 0;
	int moves[MAX_MOVES];
	if (depth == 0) {
		return 1;
	}
	
	nMoves = getMoves(pos, moves, 0, 0);
	for (i = 0; i < nMoves; ++i) {
		if (makeMove(pos, moves[i])) {
			sum += perftTest(pos, depth - 1);
			unmakeMove(pos, moves[i]);
		}
	}
	return sum;
}

