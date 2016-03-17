#include <stdio.h>

#include "defs.h"


#define SEARCH_DEPTH 6

int main(int argc, char* argv[]) {
  Board board;
  int n;
  initHashTable();
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
  //twoPlayerLoop(board);
  onePlayerLoop(board);
}


void twoPlayerLoop(Board board) {
	int move, sd, status = 0;    //0: game on, 1: white wins, 2: black wins, -1: stalemate
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
					printf("ambiguous move.\n");
					break;
				default:
					printf("%d\n", move);
					break;
			}
			scanf("%20s", input);
		}
		sd = (board.ply%2)*6;
		makeMove(&board, move);
		validateBoardState(board);
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
	validateBoardState(board);
	printf("\n");
	printBoard(board);
	printf("\n");
	while (status!=1 && status != 2 && status != -1){
		char input[21];
		if (board.ply%2) {
			printf("Black's move:\n");
			move = moveSearch(&board, SEARCH_DEPTH, &score);
			toAlg(board, move, input);
			printf("%s\n", input);
		} else {
			printf("White's move:\n");
			move = getInputMove(board);
		}
		makeMove(&board, move);
		validateBoardState(board);
		status = getGameStatus(board);
		printf("\n");
		printBoard(board);
		printf("\n");
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
		default:
			printf("Unknown game status: %d.  Exiting.\n", status);
		}
}
