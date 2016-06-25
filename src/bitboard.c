#include <stdio.h>

#include "bitboard.h"


bitboard PDIAGS[15] = {0x0100000000000000ull, 0x0201000000000000ull, 0x0402010000000000ull, 0x0804020100000000ull, 0x1008040201000000ull, 0x2010080402010000ull, 0x4020100804020100ull,
	0x8040201008040201ull, 0x0080402010080402ull, 0x0000804020100804ull, 0x0000008040201008ull, 0x0000000080402010ull, 0x0000000000804020ull, 0x0000000000008040ull, 0x0000000000000080ull
};

bitboard NDIAGS[15] = {0x0000000000000001ull, 0x0000000000000102ull, 0x0000000000010204ull, 0x0000000001020408ull, 0x0000000102040810ull, 0x0000010204081020ull, 0x001020408102040ull,
	0x0102040810204080ull, 0x0204081020408000ull, 0x0408102040800000ull, 0x0810204080000000ull, 0x1020408000000000ull, 0x2040800000000000ull, 0x4080000000000000ull, 0x8000000000000000ull
};

inline int countBits(bitboard b) {
	unsigned long long int c;
	asm (
		"popcntq %1, %0"
		: "=r" (c)
		: "r" (b)
	);
	return (int) c;
}

/*
given a bitboard, returns the first occupied position
*/
inline int bsf(bitboard b){
	unsigned long long int v;
	asm (
		"bsfq %1, %0\n\t"
		"jnz 1f\n\t"
		"movq $64, %0\n\t"
		"1:"
		: "=r" (v)
		: "r" (b)
	);
	return (int) v;
}

inline int bsr(bitboard b){
	unsigned long long int v;
	asm (
		"bsrq %1, %0\n\t"
		"jnz 1f\n\t"
		"movq $64, %0\n\t"
		"1:"
		: "=r" (v)
		: "r" (b)
	);
	return (int) v;
}

//like bsf, but returns 63 if a 1 is not found.
inline int sf(bitboard b){
	unsigned long long int v = 63;
	asm (
		"bsfq %1, %0\n\t"
		"jnz 1f\n\t"
		"movq $63, %0\n\t"
		"1:"
		: "=r" (v)
		: "r" (b)
	);
	return v;
}

//like bsr, but returns 0 if a 1 is not found
inline int sr(bitboard b){
	unsigned long long int v = 0;
	asm (
		"bsrq %1, %0\n\t"
		"jnz 1f\n\t"
		"movq $0, %0\n\t"
		"1:"
		: "=r" (v)
		: "r" (b)
	);
	return v;
}

inline int popBit(bitboard* b){
	int p = bsf(*b);
	*b &= *b - 1;
	return p;
}



void printBitboard(bitboard bits){
  int r, c;
  for (r = 7; r >= 0; r--){
  	for (c = 0; c < 8; c++){
  		int i = r*8 + c;
  		char c = ((bits >> i) & 1) ? 'x': '.';
  		printf("%c", c);
  	}
  	printf("\n");
  }
}
