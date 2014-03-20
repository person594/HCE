#include <stdio.h>

#include "defs.h"


int main(int argc, char* argv[]) {
  Board board;
  int n;
  if (argc < 4) {
  	initBoard(&board);
  } else {
  	int col, cast = 0, ep, hc = 0, fm = 1;
  	if (argv[2][0] == 'b' || argv[2][0] == 'B') {
  		col = BLACK;
  	} else {
  		col = WHITE;
  	}
  	while (*(argv[3])){
  		printf("%c\n", argv[3][0]);
  		switch (*(argv[3])) {
  			case 'K':
  				cast |= C_WK;
  				break;
  			case 'Q':
  				cast |= C_WQ;
  				break;
  			case 'k':
  				cast |= C_BK;
  				break;
  			case 'q':
  				cast |= C_BQ;
  				break;
  		}
  		++argv[3];
  	}
  	ep = getPos(argv[3][0], argv[3][1]);
  	if (argc > 4){
  		hc = strtol(argv[4], 0, 10);
  	}
  	if (argc > 5) {
  		fm = strtol(argv[5], 0, 10);
  	}
  	genBoard(&board, argv[1], col, cast, ep, hc, fm);
  }
  
  onePlayerLoop(board);
}


void twoPlayerLoop(Board board) {
	int move, sd, status = 0;		//0: game on, 1: white wins, 2: black wins, -1: stalemate
	while (!status){
		char input[21];
		printf("\n");
		printBoard(board);
		printf("\n");
		if (board.ply%2) {
			printf("Black's move:\n");
		} else {
			printf("White's move:\n");
		}
		scanf("%20s", input);
		while ((move = fromAlg(board, input)) < 0){
			switch (move) {
				case -1:
				case -2:
					printf("invalid move.\n");
					break;
				case -3:
					printf("ambiguous moce.\n");
					break;
				default:
					printf("%d\n", move);
					break;
			}
			scanf("%20s", input);
		}
		sd = (board.ply%2)*6;
		makeMove(&board, move);
		status = getGameStatus(board);
	}
	switch (status) {
		case -1:
			printf("stalemate\n");
			break;
		case 1:
			printf("White wins.\n");
			break;
		case 2:
			printf("Black wins.\n");
			break;
		}
}

void onePlayerLoop(Board board) {
	int move, score, status = 0;		//0: game on, 1: white wins, 2: black wins, -1: stalemate
	while (!status){
		char input[21];
		printf("\n");
		printBoard(board);
		printf("\n");
		if (board.ply%2) {
			printf("Black's move:\n");
			move = moveSearch(board, 5, &score);
			toAlg(board, move, input);
			printf("%s\n", input);
		} else {
			printf("White's move:\n");
			scanf("%20s", input);
			if (!strcmp(input, "quit")) {
				exit(0);
			}
			while ((move = fromAlg(board, input)) < 0){
				switch (move) {
					case -1:
					case -2:
						printf("invalid move.\n");
						break;
					case -3:
						printf("ambiguous move.\n");
						break;
					default:
						printf("%d\n", move);
						break;
				}
				scanf("%20s", input);
			}
		}
		makeMove(&board, move);
		validateBoardState(board);
		status = getGameStatus(board);
	}
	switch (status) {
		case -1:
			printf("Stalemate.\n");
			break;
		case 1:
			printf("White wins.\n");
			break;
		case 2:
			printf("Black wins.\n");
			break;
		}
}

void pollDebugMove(Board board){
	char c0, c1;
	int p0, p1, prom;
	
	printBoard(board);
	do {
		printf("initial position: ");
		scanf(" %c%c", &c0, &c1);
		p0 = getPos(c0, c1);
	} while (p0 == NO_SQUARE);
	printf("attacked?: %d\n", posAttacked(board, p0, BLACK));	
	do {
		printf("destination position: ");
		scanf(" %c%c", &c0, &c1);
		p1 = getPos(c0, c1);
	} while (p1 == NO_SQUARE);
	
	do {	
		printf("promotion piece: ");
		scanf(" %c", &c0);
		prom = getPiece(c0);
	} while(prom == EMPTY);
	
	makeMove(&board, MOV(p0, p1, prom));
	
	int p;
	for (p = WP; p <= BK; p++){
		int n;
		printf("moves for piece %d:\n", p);
		for (n = 0; board.pieces[p][n] != NO_SQUARE; n++){
			printf("\n");
			printBitboard(pieceMoves(board, p, n));
		}
	}
	
	if (validateBoardState(board)) {
		//printMoves(board);
		printf("perft 1 = %d\n", perftTest(board, 1));
		printMoves(board);
		pollDebugMove(board);
	}
}
