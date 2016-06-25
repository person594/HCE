#include <stdio.h>

#include "defs.h"
#include "transpositiontable.h"
#include "move.h"
#include "search.h"
#include "algebraic.h"
#include "polyglot.h"

#include <getopt.h>
#include <string.h>
#include <signal.h>


int interrupt_flag = 0;
void signal_handler(int signal) {
	//interrupt_flag = 1;
}

int main(int argc, char* argv[]) {
	Position pos;
	int n;
	int c, option_index;
	char *bookPath = 0;
	FILE *bookFile;
	
	signal(SIGINT, signal_handler);
	
	static struct option options[] = {
		{"book", required_argument, 0, 'b'},
		{"position", required_argument, 0, 'p'},
		//{"depth", required_argument, 0, 'd'},
		//{"record", optional_argument, 0, 'r'},
		{0,0,0,0}
	};
	initTranspositionTable();
	initPosition(&pos);
	while ((c = getopt_long(argc, argv, "b:p:", options, &option_index))!= -1) {
		switch(c) {
			case 'b':
				bookPath = optarg;
				break;
			case 'p':
				if (!optarg || !loadFEN(&pos, optarg)) {
					printf("Invalid position notation\n");
					exit(1);
				}
				break;
		}
	}
	if (bookPath) {
		bookFile = fopen(bookPath, "r");
		if (! bookFile) {
			printf("Could not open opening book file %s\n", bookPath);
			exit(1);
		}
		readPolyglotBook(bookFile);
		fclose(bookFile);
	} else {
		bookFile = fopen("/usr/share/hce/book.bin", "r");
		if (bookFile) {
			readPolyglotBook(bookFile);
			fclose(bookFile);
		}
	}
  //twoPlayerLoop(pos);
  onePlayerLoop(&pos);
  //xboardLoop(&pos);
}


void twoPlayerLoop(Position *pos) {
	int move, sd, status = 0;    //0: game on, 1: white wins, 2: black wins, -1: stalemate
	while (!status){
		char input[21];
		printf("\n");
		printPosition(pos);
		printf("\n");
		if (pos->ply%2) {
			printf("Black's move:\n");
		} else {
			printf("White's move:\n");
		}
		scanf("%20s", input);
		while ((move = fromAlg(pos, input)) < 0){
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
		sd = (pos->ply%2)*6;
		makeMove(pos, move);
		validatePosition(pos);
		status = getGameStatus(pos);
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

void onePlayerLoop(Position *pos) {
	int move, score, status = 0; //0: game on, 1: white wins, 2: black wins, -1: stalemate
	validatePosition(pos);
	printf("\n");
	printPosition(pos);
	printf("\n");
	while (status!=1 && status != 2 && status != -1){
		char input[21];
		if (pos->ply%2) {
			printf("Black's move:\n");
			move = moveSearch(pos, SEARCH_DEPTH, &score);
			toAlg(pos, move, input);
			printf("%s\n", input);
		} else {
			printf("White's move:\n");
			move = getInputMove(pos);
		}
		makeMove(pos, move);
		validatePosition(pos);
		status = getGameStatus(pos);
		printf("\n");
		printPosition(pos);
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
