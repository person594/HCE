#include <stdio.h>
#include <string.h>

#include "defs.h"

//returns -1 on bad syntax, -2 on an illegal move, and -3 on an ambiguous move
int fromAlg(Board *board, char* str) {
		int sd, p, prom = EMPTY, move = -2, sq0, sq1;
		char rank0 = 0, file0 = 0, rank, file, ch;
		bitboard b;
		
		sd = (board->ply%2) * 6;
		if (strstr(str, "O-O") == str) {  //str starts with O-O, some kind of castling
			str += 3;
			p = WK + sd;
			rank = '1' + (board->ply%2)*7;
			if (strstr(str, "-O") == str) {  //O-O-O, queenside
				str += 2;
				file = 'c';
			} else {
				file = 'g';
			}
		} else {  //not castling, normal move parsing.
			//Get the piece being moved
			p = WP + sd;
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
			// if there is another character before rank and file, i.e. a starting rank and/or file, or an x for capture.
			//if (!(str[1] >= '1' && str[1] <= '8')) {
			if (strlen(str) > 2 && strpbrk(str+2, "abcdefgh12345678")) {
				if (*str >= 'a' && *str <= 'h'){
					file0 = *str++;
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
			file = *str++;
			if (!(*str >= '1' && *str <= '8')) {
				return -1;
			}
			rank = *str++;
			if ((p == WP && rank == '8') || (p == BP && rank == '1')){ 
				prom = WQ; //default to queen promotion
			}
			if (*str == '=') {
				if (prom != EMPTY) {
					str++;
					switch (ch = *str++) {
						case 'N':
							prom = WN;
							break;
						case 'B':
							prom = WB;
							break;
						case 'R':
							prom = WR;
							break;
						case 'Q':
							prom = WQ;
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

		sq1 = 8*(rank - '1') + (file - 'a');
		
		//printf("%c%c -> %c%c\n",file0, rank0, file, rank);
		
		b = board->bits[p];
		while ((sq0 = popBit(&b)) != NO_SQUARE) {					//check all pieces of type p
			if (pieceMoves(board, p, sq0) & BIT(sq1)) {			//if the current piece can move to the target square
				if (rank0) {
					if (sq0/8 != rank0 - '1') {
						continue;
					}
				} if (file0) { 
						if (sq0%8 != file0 - 'a') {
							continue;
					}
				}
				if (move == -2) {
					int cap, ep = 0;
					cap = board->squares[sq1];
					if ((p == WP || p == BP) && cap == EMPTY && sq0%8 != sq1%8) { //en passant
						cap = BP - sd;
					}
					move = MOV(sq0, sq1, cap, prom, board->castle, board->enpas);
				} else {
					return -3;
				}
			}
		}
		
		if (move >= 0) {
			int attacked;
			makeMove(board, move);
			attacked = sqAttacked(board, bsf(board->bits[WK + sd]), WHITE + (board->ply%2));
			unmakeMove(board, move);
			if (attacked) {	//puts king in check
				return -2;
			}
		}
		return move;
}

//given the post-move board state, adds either a check (+) or checkmate (#) symbol as applicable, then
//appends a null terminator.
void terminateMoveStr(Board *board, char* str) {
	int status = getGameStatus(board);
	if (status == 1 || status == 2) {
		*str++ = '#';
	} else if (status == 3 || status == 4)  {
		*str++ = '+';
	}
	*str = 0;
}

void toAlg(Board *board, int move, char* str) {
	int to, from, prom, p, sq0, flag, sd;
	bitboard b;
	sd = 6*(board->ply%2);
	to = TO(move);
	from = FROM(move);
	prom = PROM(move);
	if (from == bsf(board->bits[WK + sd])) {		//if the moving piece is a king, possible castling notation
		if (to - from == 2) {
			strcpy(str, "O-O");
			makeMove(board, move);
			terminateMoveStr(board, str + 3);
			unmakeMove(board, move);
			return;
		} if (from - to == 2) {
			strcpy(str, "O-O-O");
			makeMove(board, move);
			terminateMoveStr(board, str + 5);
			unmakeMove(board, move);
			return;
		}
	}
	for (p = WP; p <= BK && !(board->bits[p] & BIT(from)); p++);
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
		flag = 0;	//whether two pieces can move to the same square.  1: disambiguate by file, 2: disambiguate by rank
		b = board->bits[p];
		while ((sq0 = popBit(&b)) != NO_SQUARE) {	//for each piece of the piece type being moved
			if ((pieceMoves(board, p, sq0) & BIT(to)) && sq0 != from) {
				if (sq0 % 8 != from % 8) {
					flag |= 1;
				} else {
					flag |= 2;
				}
			}
		}
		if (flag & 1) {
			*str++ = 'a' + (from % 8);
		}
		if (flag & 2) {
			*str++ = '1' + (from / 8);
		}
	}
	if (board->bits[OCCUPIED] & BIT(to)){
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
	makeMove(board, move);
	terminateMoveStr(board, str);
	unmakeMove(board, move);
}

char getSymbol(int p) {
	switch (p){
		case WP:
			return 'P';
		case WN:
			return 'N';
		case WB:
			return 'B';
		case WR:
			return 'R';
		case WQ:
			return 'Q';
		case WK:
			return 'K';
		
		case BP:
			return 'p';
		case BN:
			return 'n';
		case BB:
			return 'b';
		case BR:
			return 'r';
		case BQ:
			return 'q';
		case BK:
			return 'k';
		default:
			return ' ';
	}
}

int getPiece(char c){
	switch (c) {
		case 'P':
			return WP;
		case 'N':
			return WN;
		case 'B':
			return WB;
		case 'R':
			return WR;
		case 'Q':
			return WQ;
		case 'K':
			return WK;
			
		case 'p':
			return BP;
		case 'n':
			return BN;
		case 'b':
			return BB;
		case 'r':
			return BR;
		case 'q':
			return BQ;
		case 'k':
			return BK;
		default:
			return EMPTY;
	}
}

//polls player for a move, and returns the resulting move.  Also accepts and responds to non-move commands and input, but only returns upon receiving a valid move.
int getInputMove(Board *board) {
	#define MAXTOKENS 3
	while(1) {
		char str[21];
		char* tokens[MAXTOKENS];
		char* l;
		int numTokens;
		fgets(str, 21, stdin);
		if (!(tokens[0] = strtok(str, " \t\v\n\r\f"))) {		//no tokens on the line
			continue;
		}
		for (numTokens = 1; numTokens < MAXTOKENS && (tokens[numTokens] = strtok(0, " \t\v\n\r\f")); numTokens++);
		if (!strcmp(tokens[0], "exit")) {
			exit(0);
		}
		if (!strcmp(tokens[0], "perft")) {
			int n;
			if (numTokens == 2) {
				n = strtol(tokens[1], 0, 10);
				printf("%d\n", perftTest(board, n));
			} else {
				printf("usage : perft n\nn - number of ply to look ahead\n");
			}
			continue;
		}
		if (numTokens == 1) {
			int move;
			move = fromAlg(board, tokens[0]);
			if (move >= 0) {
				return move;
			}
			if (move == -3) {
				printf("ambiguous move.\n");
				continue;
			}
		}
		printf("invalid move / command: %s.\n", tokens[0]);
	}
	#undef MAXTOKENS
}
