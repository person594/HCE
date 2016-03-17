#include <stdio.h>

#include "defs.h"


int perftTest(Board *board, int depth){
	int sd, i, sum = 0;
	sd = (board->ply % 2) * 6;
	if (sqAttacked(board, bsf(board->bits[BK - sd]), WHITE + (board->ply % 2))){
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
		b = board->bits[p];
		while ((sq0 = popBit(&b)) != NO_SQUARE) {
			bitboard moves;
			moves = pieceMoves(board, p, sq0);
			while (moves){
				int cap, mov;
				sq1 = popBit(&moves);
				cap = board->squares[sq1];
				if ((p == WP || p == BP) && cap == EMPTY && sq0 % 8 != sq1 % 8 ){  //en passant
					cap = BP - sd;
				}
				else if ((p == WP && sq1/8 == 7) || (p == BP && sq1/8 == 0))  {  //pawn promotion
					mov = MOV(sq0, sq1, cap, WN + sd, board->castle, board->enpas);
					makeMove(board, mov);
					sum += perftTest(board, depth - 1);
					unmakeMove(board, mov);
					
					mov = MOV(sq0, sq1, cap, WB + sd, board->castle, board->enpas);
					makeMove(board, mov);
					sum += perftTest(board, depth - 1);
					unmakeMove(board, mov);
					
					mov = MOV(sq0, sq1, cap, WR + sd, board->castle, board->enpas);
					makeMove(board, mov);
					sum += perftTest(board, depth - 1);
					unmakeMove(board, mov);
					
				}
					mov = MOV(sq0, sq1, cap, WQ + sd, board->castle, board->enpas);
					makeMove(board, mov);
					sum += perftTest(board, depth - 1);
					unmakeMove(board, mov);
				
				/*
				if (!validateBoardState(b2)) {
					exit(0);
				}
				*/
			}
		}
	}
	return sum;
}

int printMoves(Board *board){
	int sd, i;
	sd = (board->ply % 2) * 6;
	for (i = 0; i < 6; i++){
		int n, p, sq0, sq1;
		bitboard b;
		p = sd + i;
		b = board->bits[p];
		while ((sq0 = popBit(&b)) != NO_SQUARE) {
			bitboard moves;
			moves = pieceMoves(board, p, n);
			while (moves){
				int cap, move;
				sq1 = popBit(&moves);
				cap = board->squares[sq1];
				if ((p == WP || p == BP) && cap == EMPTY && sq0 % 8 != sq1 % 8) { //en passant
					cap = BP - sd;
				}
				if ((p == WP && sq1/8 == 7) || (p == BP && sq1/8 == 0))	{	//pawn promotion
					
					move = MOV(sq0, sq1, cap, WN, board->castle, board->enpas);
					makeMove(board, move);
					if (!sqAttacked(board, bsf(board->bits[WK + sd]), WHITE + (board->ply % 2))){
						printf("%d: %d-%d\n", p, sq0, sq1);
					}
					unmakeMove(board, move);
					
					
					move = MOV(sq0, sq1, cap, WB, board->castle, board->enpas);
					makeMove(board, move);
					if (!sqAttacked(board, bsf(board->bits[WK + sd]), WHITE + (board->ply % 2))){
						printf("%d: %d-%d\n", p, sq0, sq1);
					}
					unmakeMove(board, move);
					
					
					move = MOV(sq0, sq1, cap, WR, board->castle, board->enpas);
					makeMove(board, move);
					if (!sqAttacked(board, bsf(board->bits[WK + sd]), WHITE + (board->ply % 2))){
						printf("%d: %d-%d\n", p, sq0, sq1);
					}
					unmakeMove(board, move);

				}
				move = MOV(sq0, sq1, cap, WQ, board->castle, board->enpas);
				makeMove(board, move);
				if (!sqAttacked(board, bsf(board->bits[WK + sd]), WHITE + (board->ply % 2))){
					printf("%d: %d-%d\n", p, sq0, sq1);
				}
				unmakeMove(board, move);
				//printf("sum = %d\n", sum);
			}
		}
	}
}
