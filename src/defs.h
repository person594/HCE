#ifndef DEFS_H
#define DEFS_H

#include <stdlib.h>
#include <stdio.h>

#include "position.h"

#define P_VAL 100
#define N_VAL 320
#define B_VAL 330
#define R_VAL 500
#define Q_VAL 900
#define K_VAL 90000

#define MAX_VAL (2*K_VAL)
#define MIN_VAL (-MAX_VAL)

extern int VAL[];
extern int interrupt_flag;

#define ABS(n) (n >= 0 ? n : -n)

void twoPlayerLoop(Position *);
void onePlayerLoop(Position *);
int getGameStatus(Position *);


int eval(Position *pos);

int fromAlg(Position *, char*);
void toAlg(Position *, int, char*);
char getSymbol(int);


typedef struct {
	char *name;
	int i_value;
	char *s_value;
	int accepted;
} feature;


char *readLine(void);

void xboardLoop(Position *pos);

#endif
