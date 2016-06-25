#include "search.h"
#include "move.h"
#include "movegen.h"
#include "transpositiontable.h"
#include "defs.h"

int quiescence(Position *pos, int alpha, int beta) {
	int numMoves, i, moves[MAX_MOVES], bestMove = 0;
	int nodeType = UPPER;
	int standingPat;
	#define DELTA 200
	standingPat = eval(pos); 
	if (standingPat >= beta) {
		return beta;
	}
	if (standingPat > alpha) {  //null move heuristic
		alpha = standingPat;
	}
	if (standingPat + Q_VAL < alpha) { //futility check
		return alpha;
	}
	if (!pos->bits[WK] || !pos->bits[BK]) {
		return standingPat;
	}
	getTableBounds(pos, &alpha, &beta, 0);
	if (alpha >= beta) return alpha;
	numMoves = getMoves(pos, moves, 1, 1);
	orderMoves(pos, numMoves, moves);

	for (i = 0; i < numMoves; ++i) {
		int score;
		//delta pruning
		if (ABS(VAL[CAP(moves[i])]) + DELTA < alpha) continue;
		if (makeMove(pos, moves[i])) {
			score = -quiescence(pos, -beta, -alpha);
			unmakeMove(pos, moves[i]);
			if (score >= beta) {
				nodeType = LOWER;
				addToTable(pos, beta, 0, nodeType, moves[i]);
				return beta;
			}
			if (score > alpha) {
				alpha = score;
				bestMove = moves[i];
			}
		}
	}
	addToTable(pos, beta, 0, nodeType, bestMove);
	return alpha;
}

int moveSearch(Position *pos, int depth, int *score) {
	int numMoves, i, moves[MAX_MOVES];
	int bestMove;
	int current_depth;
	interrupt_flag = 0;
	for (current_depth = 0; current_depth < depth; ++current_depth) {
		int alpha = MIN_VAL;
		numMoves = getMoves(pos, moves, 1, 0);
		orderMoves(pos, numMoves, moves);
		for (i = 0; i < numMoves; ++i) {
			int score;
			if (makeMove(pos, moves[i])) {
				score = -alphaBeta(pos, MIN_VAL, -alpha, current_depth);
				unmakeMove(pos, moves[i]);
				if (score > alpha) {
					alpha = score;
					bestMove = moves[i];
				}
			}
		}
	}
	return bestMove;
}

int alphaBeta(Position *pos, int alpha, int beta, int depthleft) {
	int numMoves, i, moves[MAX_MOVES];
	int nodeType = UPPER;
	int bestMove = 0;
	int legalMove = 0;
	getTableBounds(pos, &alpha, &beta, depthleft);
	if (interrupt_flag) return alpha;
	if (alpha >= beta) return alpha;
	if (depthleft <= 0 || !pos->bits[WK] || !pos->bits[BK]) {
		return quiescence(pos, alpha, beta);
	}
	numMoves = getMoves(pos, moves, 1, 0);
	orderMoves(pos, numMoves, moves);
	for (i = 0; i < numMoves; ++i) {
		int score;
		if (makeMove(pos, moves[i])) {
			legalMove = 1;
			score = -alphaBeta(pos, -beta, -alpha, depthleft - 1);
			unmakeMove(pos, moves[i]);
			if (score >= beta) {
				nodeType = LOWER;
				addToTable(pos, beta, depthleft, nodeType, moves[i]);
				return beta;
			}
			if (score > alpha) {
				alpha = score;
				bestMove = moves[i];
				nodeType = EXACT;
			}
		}
	}
	if (!legalMove) {
		if (inCheck(pos, (pos->ply%2 == 0 ? WHITE : BLACK))) return alpha;
		else return 0;
	}
	addToTable(pos, alpha, depthleft, nodeType, bestMove);
	return alpha;
}
