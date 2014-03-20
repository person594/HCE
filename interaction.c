#include <stdio.h>
#include <string.h>

#include "defs.h"

//returns -1 on bad syntax, -2 on an illegal move, and -3 on an ambiguous move
int fromAlg(Board board, char* str) {
		int n, sd, p, prom, move = -2, square;
		char rank0 = 0, row0 = 0, rank, row, ch;
		
		sd = (board.ply%2) * 6;
		if (strstr(str, "O-O") == str) {	//str starts with O-O, some kind of castling
			str +=3;
			p = WK + sd;
			rank = '1' + (board.ply%2)*7;
			if (strstr(str, "-O")) {				//O-O-O, queenside
				str += 2;
				row = 'c';
			} else {
				row = 'g';
			}
		} else {													//not castling, normal move parsing.
			//Get the piece being moved
			p = WP + sd;
			prom = WQ + sd;		//default to queen promotion
			switch (*str) {
				case  'N':
					p = WN + sd;
					str++;
					break;
				case 'B':
					p = WB + sd;
					str++;
					break;
				case 'R':
					p = WR + sd;
					str++;
					break;
				case 'Q':
					p = WQ + sd;
					str++;
					break;
				case 'K':
					p = WK + sd;
					str++;
					break;
				case 0:
					return -1;
			}
			// if there is another character before rank and row, i.e. a starting rank and/or row, or an x for capture.
			//if (!(str[1] >= '1' && str[1] <= '8')) {
			if (strlen(str) > 2 && strpbrk(str+2, "abcdefgh12345678")) {
				if (*str >= 'a' && *str <= 'h'){
					row0 = *str++;
				}
				if (*str >= '1' && *str <= '8') {
					rank0 = *str++;
				}
				if (*str == 'x'){
					++str;
				}
			}
			if (!(*str >= 'a' && *str <= 'h')) {
				return -1;
			}
			row = *str++;
			if (!(*str >= '1' && *str <= '8')) {
				return -1;
			}
			rank = *str++;
			if (*str == '=') {
				if (p == WP | p == BP) {
					str++;
					switch (ch = *str) {
						case 'N':
							prom = WN + sd;
							break;
						case 'B':
							prom = WB + sd;
							break;
						case 'R':
							prom = WR + sd;
							break;
						case 'Q':
							prom = WQ + sd;
							break;
						default:
							return -1;
					}
				}
				else {
					return -1;
				}
			}
		}
		while (ch = *str++) {
			if (!(ch == '+' || ch == '#' || ch == '!' || ch == '?')){
				return -1;
			}
		}
		
		square = 8*(rank - '1') + (row - 'a');
		
		
		for (n = 0; n < 10 && board.pieces[p][n] != NO_SQUARE; ++n) {
			if (pieceMoves(board, p, n) & BIT(square)) {
				if (rank0) {
					if (board.pieces[p][n]/8 != rank0 - '1') {
						continue;
					}
				} if (row0) {
						if (board.pieces[p][n]%8 != row0 - 'a') {
							continue;
					}
				}
				if (move == -2) {
					move = MOV(board.pieces[p][n], square, prom);
				} else {
					return -3;
				}
			}
		}
		
		if (move >= 0) {
			Board b2 = board;
			makeMove(&b2, move);
			if (posAttacked(b2, b2.pieces[WK + sd][0], WHITE + (b2.ply%2))) {	//puts king in check
				return -2;
			}
		}
		
		return move;
}


void toAlg(Board board, int move, char* str) {
	int to, from, prom, p, n, flag, sd;
	sd = 6*(board.ply%2);
	to = TO(move);
	from = FROM(move);
	prom = PROM(move);
	if (board.pieces[WK+sd][0] == from) {		//if the moving piece is a king, possible castling notation
		if (to - from == 2) {
			strcpy(str, "O-O");
			return;
		} if (from - to == 2) {
			strcpy(str, "O-O-O");
			return;
		}
	}
	for (p = WP; p <= BK && !(board.bits[p] & BIT(from)); p++);
	switch (p) {
		case WN:
		case BN:
			*str++ = 'N';
			break;
		case WB:
		case BB:
			*str++ = 'B';
			break;
		case WR:
		case BR:
			*str++ = 'R';
			break;
		case WQ:
		case BQ:
			*str++ = 'Q';
			break;
		case WK:
		case BK:
			*str++ = 'K';
			break;
	}
	if (p == WP || p == BP) {
		if (to%8 != from%8) {
			*str++ = 'a' + (from % 8);
		}
	} else {
		flag = 0;	//whether two pieces can move to the same square
		for (n = 0; n < 10 && board.pieces[p][n] != NO_SQUARE; n++) {	//for each of the piece being moves
			if ((pieceMoves(board, p, n) & BIT(to)) && board.pieces[p][n] != from) {
				if (board.pieces[p][n] % 8 == from) {
					flag |= 2;
				} else {
					flag |= 1;
				}
			}
			if (flag & 1) {
				*str++ = 'a' + (from % 8);
			}
			if (flag & 2) {
				*str++ = '1' + (from / 8);
			}
		}
	}
	if (board.bits[OCCUPIED] & BIT(to)){
		*str++ = 'x';
	}
	*str++ = 'a' + (to % 8);
	*str++ = '1' + (to / 8);
	if ((p == WP || p == WP) && to/8 == 7) {
		*str++ = '=';
		switch (prom) {
			case WN:
			case BN:
				*str++ = 'N';
				break;
			case WB:
			case BB:
				*str++ = 'B';
				break;
			case WR:
			case BR:
				*str++ = 'R';
				break;
			case WQ:
			case BQ:
				*str++ = 'Q';
				break;
			case WK:
			case BK:
				*str++ = 'K';
				break;
		}
	}
	*str = 0;
}

