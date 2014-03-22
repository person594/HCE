#include <stdio.h>

#include "defs.h"


int perftTest(Board board, int depth){
	int sd, i, sum = 0;
	sd = (board.ply % 2) * 6;
	if (posAttacked(board, board.pieces[BK - sd][0], WHITE + (board.ply % 2))){
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
		int n, p, p0, p1;
		p = sd + i;
		for (n = 0; n < 10 && (p0 = board.pieces[p][n]) != NO_SQUARE; n++) {
			Board b2;
			bitboard moves;
			moves = pieceMoves(board, p, n);
			while (moves){
				p1 = popBit(&moves);
				b2 = board;
				if ((p == WP && p1/8 == 7) || (p == BP && p1/8 == 0))	{	//pawn promotion
					makeMove(&b2, MOV(p0, p1, WN + sd));
					//printf("%d: %d-%d\n", p, p0, p1);
					sum += perftTest(b2, depth - 1);
					
					b2 = board;
					makeMove(&b2, MOV(p0, p1, WB + sd));
					//printf("%d: %d-%d\n", p, p0, p1);
					sum += perftTest(b2, depth - 1);
					
					b2 = board;
					makeMove(&b2, MOV(p0, p1, WR + sd));
					//printf("%d: %d-%d\n", p, p0, p1);
					sum += perftTest(b2, depth - 1);
					
					b2 = board;
					makeMove(&b2, MOV(p0, p1, WQ + sd));
					//printf("%d: %d-%d\n", p, p0, p1);
					sum += perftTest(b2, depth - 1);
				} else {
					makeMove(&b2, MOV(p0, p1, 0));
					//printf("%d: %d-%d\n", p, p0, p1);
					sum += perftTest(b2, depth - 1);
					//printf("sum = %d\n", sum);
				}
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
		int n, p, p0, p1;
		p = sd + i;
		for (n = 0; n < 10 && (p0 = board.pieces[p][n]) != NO_SQUARE; n++) {
			Board b2;
			bitboard moves;
			moves = pieceMoves(board, p, n);
			while (moves){
				p1 = popBit(&moves);
				if ((p == WP && p1/8 == 7) || (p == BP && p1/8 == 0))	{	//pawn promotion
					makeMove(&b2, MOV(p0, p1, WN + sd));
					if (!posAttacked(b2, b2.pieces[WK + sd][0], WHITE + (b2.ply % 2))){
						printf("%d: %d-%d\n", p, p0, p1);
					}
					
					b2 = board;
					makeMove(&b2, MOV(p0, p1, WB + sd));
					if (!posAttacked(b2, b2.pieces[WK + sd][0], WHITE + (b2.ply % 2))){
						printf("%d: %d-%d\n", p, p0, p1);
					}
					
					b2 = board;
					makeMove(&b2, MOV(p0, p1, WR + sd));
					if (!posAttacked(b2, b2.pieces[WK + sd][0], WHITE + (b2.ply % 2))){
						printf("%d: %d-%d\n", p, p0, p1);
					}
					
					b2 = board;
					makeMove(&b2, MOV(p0, p1, WQ + sd));
					if (!posAttacked(b2, b2.pieces[WK + sd][0], WHITE + (b2.ply % 2))){
						printf("%d: %d-%d\n", p, p0, p1);
					}
				}
				b2 = board;
				makeMove(&b2, MOV(p0, p1, 0));
				if (!posAttacked(b2, b2.pieces[WK + sd][0], WHITE + (b2.ply % 2))){
						printf("%d: %d-%d\n", p, p0, p1);
					}
				//printf("sum = %d\n", sum);
			}
		}
	}
}
