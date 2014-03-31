#include <stdio.h>

#include "defs.h"


int perftTest(Board board, int depth){
	int sd, i, sum = 0;
	sd = (board.ply % 2) * 6;
	if (sqAttacked(board, bsf(board.bits[BK - sd]), WHITE + (board.ply % 2))){
		//printf("-1\n");
		return 0;
	}
	if (depth == 0) {
		//printf("\n");
		//printBoard(board);
		//printf("1\n");
		return 1;
	}
	for (i = 0; i < 6; i++){
		int n, p, sq0, sq1;
		bitboard b;
		p = sd + i;
		b = board.bits[p];
		while ((sq0 = popBit(&b)) != NO_SQUARE) {
			Board b2;
			bitboard moves;
			moves = pieceMoves(board, p, sq0);
			while (moves){
				sq1 = popBit(&moves);
				b2 = board;
				if ((p == WP && sq1/8 == 7) || (p == BP && sq1/8 == 0))	{	//pawn promotion
					makeMove(&b2, MOV(sq0, sq1, WN + sd));
					sum += perftTest(b2, depth - 1);
					
					b2 = board;
					makeMove(&b2, MOV(sq0, sq1, WB + sd));
					sum += perftTest(b2, depth - 1);
					
					b2 = board;
					makeMove(&b2, MOV(sq0, sq1, WR + sd));
					sum += perftTest(b2, depth - 1);
					
				}
				b2 = board;
				makeMove(&b2, MOV(sq0, sq1, WQ + sd));
				sum += perftTest(b2, depth - 1);
				
				if (!validateBoardState(b2)) {
					exit(0);
				}
			}
		}
	}
	return sum;
}

int printMoves(Board board){
	int sd, i;
	sd = (board.ply % 2) * 6;
	for (i = 0; i < 6; i++){
		int n, p, sq0, sq1;
		bitboard b;
		p = sd + i;
		b = board.bits[p];
		while ((sq0 = popBit(&b)) != NO_SQUARE) {
			Board b2;
			bitboard moves;
			moves = pieceMoves(board, p, n);
			while (moves){
				sq1 = popBit(&moves);
				if ((p == WP && sq1/8 == 7) || (p == BP && sq1/8 == 0))	{	//pawn promotion
					makeMove(&b2, MOV(sq0, sq1, WN + sd));
					if (!sqAttacked(b2, bsf(b2.bits[WK + sd]), WHITE + (b2.ply % 2))){
						printf("%d: %d-%d\n", p, sq0, sq1);
					}
					
					b2 = board;
					makeMove(&b2, MOV(sq0, sq1, WB + sd));
					if (!sqAttacked(b2, bsf(b2.bits[WK + sd]), WHITE + (b2.ply % 2))){
						printf("%d: %d-%d\n", p, sq0, sq1);
					}
					
					b2 = board;
					makeMove(&b2, MOV(sq0, sq1, WR + sd));
					if (!sqAttacked(b2, bsf(b2.bits[WK + sd]), WHITE + (b2.ply % 2))){
						printf("%d: %d-%d\n", p, sq0, sq1);
					}
					
					b2 = board;
					makeMove(&b2, MOV(sq0, sq1, WQ + sd));
					if (!sqAttacked(b2, bsf(b2.bits[WK + sd]), WHITE + (b2.ply % 2))){
						printf("%d: %d-%d\n", p, sq0, sq1);
					}
				}
				b2 = board;
				makeMove(&b2, MOV(sq0, sq1, 0));
				if (!sqAttacked(b2, bsf(b2.bits[WK + sd]), WHITE + (b2.ply % 2))){
						printf("%d: %d-%d\n", p, sq0, sq1);
					}
				//printf("sum = %d\n", sum);
			}
		}
	}
}
