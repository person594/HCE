#include <stdio.h>
#include <string.h>

#include "defs.h"

feature features[] = {
	{"variants", 0, "normal", 0},
	{"playother", 1, 0, 0},
	{"colors", 0, 0, 0},
	{"san", 1, 0, 0}
};

int n_features = sizeof(features) / sizeof(feature);

void xboardLoop(Board *board) {
	int version = 1;
	int player = 2; //don't make moves
	setbuf(stdout, NULL);
	while (1) {
		char *line;
		char *word;
		line = readLine();
		word = strtok(line, " ");
		if (strcmp(word, "xboard") == 0) {
			; //no action required
		} else if (strcmp(word, "protover") == 0) {
			int i;
			word = strtok(NULL, " ");
			version = atol(word);
			for (i = 0; i < n_features; ++i) {
				if (features[i].s_value) {
					printf("feature %s=\"%s\"\n", features[i].name, features[i].s_value);
				} else {
					printf("feature %s=%d\n", features[i].name, features[i].i_value);
				}
			}
		} else if (strcmp(word, "accepted") == 0) {
			int i;
			word = strtok(NULL, " ");
			for (i = 0; i < n_features; ++i) {
				if (strcmp(word, features[i].name) == 0) {
					features[i].accepted = 1;
				}
			}
		} else if (strcmp(word, "rejected") == 0) {
			int i;
			word = strtok(NULL, " ");
			for (i = 0; i < n_features; ++i) {
				if (strcmp(word, features[i].name) == 0) {
					features[i].accepted = 0;
				}
			}
		} else if (strcmp(word, "new") == 0) {
			initBoard(board);
			player = 1;
		} else if (strcmp(word, "variant") == 0) {
			//uh oh, variants not supported yet
			exit(1);
		} else if (strcmp(word, "quit") == 0) {
			exit(0);
		} else if (strcmp(word, "random") == 0) {
			;//not supported, nop for the time being
		} else if (strcmp(word, "force") == 0) {
			player = 2; //force mode
		} else if (strcmp(word, "go") == 0) {
			player = board->ply % 2;
		} else if (strcmp(word, "playother") == 0) {
			player = 1 - (board->ply % 2);
		} else if (strcmp(word, "level") == 0) {
			//TODO: add time controls
			;
		} else if (strcmp(word, "st") == 0) {
			;
		} else if (strcmp(word, "sd") == 0) {
			//TODO: set depth
			;
		} else {
			int move;
			move = fromAlg(board, word);
			if (move < -1) {
				printf("Illegal move: %s\n", word);
			} else if (move > 0) {
				makeMove(board, move);
			}
		}
		if (player == board->ply % 2) {
			char moveStr[21];
			int move, score;
			move = moveSearch(board, SEARCH_DEPTH, &score);
			toAlg(board, move, moveStr);
			makeMove(board, move);
			printf("move %s\n", moveStr);
		}
	}
}

char *readLine() {
	static int n = 256;
	static char *s = 0;
	char *p;
	int c;
	if (s == 0) {
		s = (char *)malloc(n);
	}
	p = s;
	while (c = getchar()) {
		if (c == -1 || c == '\n') {
			c = 0;
		}
		*p++ = c;
		if (c == 0) return s;
		if (p - s == n) {
			n*=2;
			s = realloc(s, n);
		}
	}
}
