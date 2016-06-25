#ifndef BITBOARD_H
#define BITBOARD_H

typedef unsigned long long int bitboard;

extern bitboard PDIAGS[15];
extern bitboard NDIAGS[15];

#define R_1 0x00000000000000FFull
#define R_2 0x000000000000FF00ull
#define R_3 0x0000000000FF0000ull
#define R_4 0x00000000FF000000ull
#define R_5 0x000000FF00000000ull
#define R_6 0x0000FF0000000000ull
#define R_7 0x00FF000000000000ull
#define R_8 0xFF00000000000000ull

#define R_A 0x0101010101010101ull
#define R_B 0x0202020202020202ull
#define R_C 0x0404040404040404ull
#define R_D 0x0808080808080808ull
#define R_E 0x1010101010101010ull
#define R_F 0x2020202020202020ull
#define R_G 0x4040404040404040ull
#define R_H 0x8080808080808080ull

#define CENTER 0x0000001818000000ull

#define D_P 0x8040201008040201ull
#define D_N 0x0102040810204080ull

//bitboards for the moves of a knight and a king, each positioned at e5.
#define KNIGHTMOVE 0x0028440044280000ull
#define KINGMOVE 0x0000382838000000ull




#define BIT(pos) ((pos > 63 || pos < 0) ? 0ull : 1ull << pos)

#define RANKOF(pos) (R_1<<(pos&0x38))
#define FILEOF(pos) (R_A<<(pos&0x7))
#define PDIAG(pos) PDIAGS[7 + pos%8 - pos/8]
#define NDIAG(pos) NDIAGS[pos%8 + pos/8]


//macros to translate bitboards in a given direction
#define NORTH(b, n) (b<<(8*n))
#define SOUTH(b, n) (b>>(8*n))
//#define EAST(b) ((b & ~R_H)<<1)
//#define WEST(b) ((b & ~R_A)>>1)
#define EAST(b, n) ((b&((R_A<<(8-n)) - R_A))<<n)
#define WEST(b, n) ((b&~((R_A<<n) - R_A))>>n)
#define TRANS(b, x, y) (y >= 0 ? NORTH((x >= 0 ? EAST(b, x) : WEST(b, -x)), y) : SOUTH((x >= 0 ? EAST(b, x) : WEST(b, -x)), -y))

#define SAMERANK(p0, p1) !((p0^p1) >> 3)
#define SAMEFILE(p0, p1) !((p0^p1) & 0x7)


#define RANKDIF(p0, p1) ((p0>>3) - (p1>>3))
#define FILEDIF(p0, p1) ((p0 & 0x7) - (p1 & 0x7))



#endif
