#include <string.h>

#include "defs.h"
#include "position.h"
#include "algebraic.h"


int ISBLACK[] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1};
int COLOR[] = {WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, WHITE, BLACK, OCCUPIED, EMPTY};


void clearPosition(Position *pos) {
	int n, p;
	for (n = 0; n < 15; n++){
		pos->bits[n] = 0ull;
	}
	for (n = 0; n < 64; n++) {
		pos->squares[n] = EMPTY;
	}
	pos->bits[EMPTY] = ~pos->bits[OCCUPIED];
	pos->ply = 0;
	pos->enpas = NO_SQUARE;
	pos->castle  = C_WK | C_WQ | C_BK | C_BQ;
	
	pos->score = 0;
	pos->ephash = 0ull;
	pos->hash = hashPosition(pos);
}



void initPosition(Position* pos){
	int p, n;
	
	pos->ply = 0;
	pos->enpas = NO_SQUARE;
	pos->castle = C_WK | C_WQ | C_BK | C_BQ;
	
	pos->bits[WP] = R_2;
	pos->bits[WN] = R_1 & (R_B | R_G);
	pos->bits[WB] = R_1 & (R_C | R_F);
	pos->bits[WR] = R_1 & (R_A | R_H);
	pos->bits[WQ] = R_1 & R_D;
	pos->bits[WK] = R_1 & R_E;
	
	pos->bits[BP] = R_7;
	pos->bits[BN] = R_8 & (R_B | R_G);
	pos->bits[BB] = R_8 & (R_C | R_F);
	pos->bits[BR] = R_8 & (R_A | R_H);
	pos->bits[BQ] = R_8 & R_D;
	pos->bits[BK] = R_8 & R_E;
	
	pos->bits[WHITE] = R_1 | R_2;
	pos->bits[BLACK] = R_7 | R_8;
	pos->bits[OCCUPIED] = (pos->bits[WHITE]) | (pos->bits[BLACK]);
	pos->bits[EMPTY] = ~(pos->bits[OCCUPIED]);
	
	for (n = 0; n < 64; n++) {
		if (n/8 == 0 || n/8 == 7) {
			switch (n%8) {
				case 0:
				case 7:
					pos->squares[n] = WR;
					break;
				case 1:
				case 6:
					pos->squares[n] = WN;
					break;
				case 2:
				case 5:
					pos->squares[n] = WB;
					break;
				case 3:
					pos->squares[n] = WQ;
					break;
				case 4:
					pos->squares[n] = WK;
					break;
			}
		} else if (n/8 == 1 || n/8 == 6) {
			pos->squares[n] = WP;
		} else {
			pos->squares[n] = EMPTY;
		}
		if (n/8 >= 6) {
			pos->squares[n] += BP;
		}
	}
	
	pos->score = 0;
	pos->ephash = 0ull;
	pos->hash = hashPosition(pos);
}

//generates a position given Forsyth-Edwards Notation
int loadFEN(Position* pos, char* fen){
	int i, r, f;
	char *pieces, *side, *castling, *enpassant, *halfmove, *fullmove;
	char str[512];
	Position newPos; //make our position locally, then copy it over at the end
	strncpy(str, fen, 511);
	pieces = strtok(str, " ");
	side = strtok(NULL, " ");
	if (!side) return 0;
	castling = strtok(NULL, " ");
	if (!castling) return 0;
	enpassant = strtok(NULL, " ");
	if (!enpassant) return 0;
	halfmove = strtok(NULL, " ");
	if (!halfmove) return 0;
	fullmove = strtok(NULL, " ");
	if (!fullmove) return 0;
	clearPosition(&newPos);
	newPos.ply = 2 * (atol(fullmove) - 1);
	if (newPos.ply < 0) return 0;
	if (strcmp(side, "b") == 0) {
		newPos.ply++;
	} else if (strcmp(side, "w") != 0) {
		return 0;
	}
	newPos.castle = 0;
	if (strcmp(castling, "-") != 0) {
		if (*castling == 'K' || *castling == 'A') {
			newPos.castle |= C_WK;
			++castling;
		}
		if (*castling == 'Q' || *castling == 'H') {
			newPos.castle |= C_WQ;
			++castling;
		}
		if (*castling == 'k' || *castling == 'a') {
			newPos.castle |= C_BK;
			++castling;
		}
		if (*castling == 'q' || *castling == 'h') {
			newPos.castle |= C_BQ;
			++castling;
		}
		if (*castling) return 0;
	}
	if (strcmp(enpassant, "-") != 0) {
		int file, rank;
		file = enpassant[0] - 'a';
		if (file < 0 || file >= 8) return 0;
		rank = enpassant[1] - '1';
		if (rank != 2 && rank != 5) return 0;
		newPos.enpas = 8*rank + file;
	}
	newPos.ephash = EPHASH(&newPos);
	for (i = 0, r = 7, f = 0; pieces[i]; ++i) {
		int p;
		char ch;
		ch = pieces[i];
		if (ch == '/'){
			if (f != 8) return 0;
			if (--r < 0) return 0;
			f = 0;
		} else if (ch > '0' && ch <= '8'){
			f += (ch - '0');
		} else {
			if (f > 7) return 0;
			p = getPiece(ch);
			if (p == EMPTY) {
				return 0;
			}
			newPos.bits[p] |= BIT(r*8 + f);
			newPos.bits[OCCUPIED] |= BIT(r*8 + f);
			newPos.bits[WHITE + ISBLACK[p]] |= BIT(r*8 + f);
			newPos.score += VAL[p];
			newPos.squares[r*8 + f] = p;
			++f;
		}
	}
	if (r != 0 && f != 8) return 0;
	newPos.bits[EMPTY] = ~newPos.bits[OCCUPIED];
	newPos.hash = hashPosition(&newPos);
	*pos = newPos;
	return 1;
}

//This function is not fast.  It should be used to generate an initial hashcode for a new position.  If modifying an existing position, the board's hashcode should be modified on the fly.
u64 hashPosition(Position *pos) {
	u64 hash = 0ull;
	int sq;
	for (sq = 0; sq < 64; sq++) {
		if (pos->squares[sq] != EMPTY) {
			hash ^= PHASH(pos->squares[sq], sq);
		}
	}
	//en passant
	hash ^= EPHASH(pos);
	//castling
	int castle;
	for (castle = 8; castle; castle >>= 1) {
		if (castle & pos->castle) {
			hash ^= CASTLEHASH(castle);
		}
	}
	//ply
	if (pos->ply%2 == 0) {
		hash ^= WHITETURNHASH;
	}
	return hash;
}


int validatePosition(Position *pos) {
	int p, n, squares[64], score = 0;
	bitboard occupied = 0, white = 0, black = 0;
	
	for (n = 0; n < 64; n++) {
		squares[n] = EMPTY;
	}
	
	for (p = WP; p <= BK; p++){
		bitboard pbits;
		pbits = pos->bits[p];
		if (occupied & pbits) {
			printf("Multiple pieces at a single square.\n");
			return 0;
		}
		occupied |= pbits;
		if (p < BP) {
			white |= pbits;
		} else {
			black |= pbits;
		}
		score += VAL[p] * countBits(pbits);
		while (pbits) {
			squares[popBit(&pbits)] = p;
		}
		
	}
	//generate the rest of our bitboards:
	
	if (white != pos->bits[WHITE] || black != pos->bits[BLACK] || occupied != pos->bits[OCCUPIED] || ~occupied != pos->bits[EMPTY]) {
		printf("Incorrect derived bitboards.\n");
		printf("white:\n");
		printBitboard(pos->bits[WHITE]);
		printf("\nblack:\n");
		printBitboard(pos->bits[BLACK]);
		printf("\noccupied:\n");
		printBitboard(pos->bits[OCCUPIED]);
		printf("\nempty:\n");
		printBitboard(pos->bits[EMPTY]);
		return 0;
	}
	
	if (score != pos->score) {
		printf("Incorrect score, expected: %d, actual: %d\n", score, pos->score);
		return 0;
	}
	
	for (n = 0; n < 64; n++) {
		if (squares[n] != pos->squares[n]) {
			printf("Incorrect piece buffer.");
			return 0;
		}
	}
	if (pos->ephash != EPHASH(pos)) {
		printf("Incorrect en passant hash code, expected: %llx, actual: %llx.\n", EPHASH(pos), pos->ephash);
		return 0;
	}
	
	if (pos->hash != hashPosition(pos)) {
		printf("Incorrect hash code, expected: %llx, actual: %llx.\n", hashPosition(pos), pos->hash);
		return 0;
	}
	
	return 1;
}

int comparePositions(Position *pos1, Position *pos2) {
	int p, sq;
	for (p = 0; p < 15; ++p) {
		if (pos1->bits[p] !=pos2->bits[p]) {
			printf("Bitboard mismatch for piece %d\n", p);
			printBitboard(pos1->bits[p]);
			printf("\n");
			printBitboard(pos2->bits[p]);
			printf("\n");
			return 0;
		}
	}
	if (pos1->ply != pos2->ply) {
		printf("Ply mismatch\n%d\n%d\n", pos1->ply, pos2->ply);
		return 0;
	}
	
	if (pos1->enpas != pos2->enpas) {
		printf("En passant square mismatch\n%d\n%d\n", pos1->enpas, pos2->enpas);
		return 0;
	}
	
	if (pos1->castle != pos2->castle) {
		printf("Castling rights mismatch\n%x\n%x\n", pos1->castle, pos2->castle);
		return 0;
	}
  if (pos1->score != pos2->score) {
		printf("Score mismatch\n%d\n%d\n", pos1->score, pos2->score);
		return 0;
	}
	for (sq = 0; sq < 64; ++sq) {
		if (pos1->squares[sq] != pos2->squares[sq]) {
			printf("Square mismatch\n");
			printPosition(pos1);
			printPosition(pos2);
			return 0;
		}
	}
	if (pos1->hash != pos2->hash) {
		printf("Hash mismatch\n%x\n%x\n", pos1->hash, pos2->hash);
		return 0;
	}
	if (pos1->ephash != pos2->ephash) {
		printf("En passant hash mismatch\n%x\n%x\n", pos1->hash, pos2->hash);
		return 0;
	}
	return 1;
}

void printPosition(Position *pos){
	char chars[64];
	int p, n, r, c;
	for (n = 0; n < 64; n++){
		chars[n] = ' ';
	}
	for (p = 0; p < 12; p++) {
		int sq;
		bitboard b = pos->bits[p];
		while ((sq = popBit(&b)) != NO_SQUARE) {
			chars[sq] = getSymbol(p);
		}
	}
	/* en passant square
	if (pos->enpas != NO_SQUARE) {
		chars[pos->enpas] = 'x';
	}
	*/
	//printf("%.8s\n%.8s\n%.8s\n%.8s\n%.8s\n%.8s\n%.8s\n%.8s\n", &chars[56], &chars[48], &chars[40], &chars[32], &chars[24], &chars[16], &chars[8], &chars[0]);
	for (r = 7; r >= 0; r--) {
		for (c = 0; c < 8; c++) {
			int at = 1, fg = 31, bg;
			if (r%2 == c%2) {
				bg = 45;
			} else {
				bg = 47;
			}
			//fg = 77 - bg;
			printf("%c[%d;%d;%dm",27,at,fg,bg);
			printf("%c ", chars[8*r + c]);
		}
		printf("%c[0m", 27);
		printf("\n");
	}
	//printf("%llx\n", pos->hash);
}
