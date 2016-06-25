#ifndef HASH_H
#define HASH_H

typedef unsigned long long int u64;

extern const u64 hashvals[781];

#define POLYGLOTPIECE(p) (2*((p)%6) + 1 - (p)/6)
#define PHASH(p, sq) (hashvals[64*POLYGLOTPIECE(p) + sq])
#define EPHASHRAW(sq) (hashvals[772+(sq)%8])
#define EPHASH(board) ((board)->enpas == NO_SQUARE ? 0ull : \
                      (board)->ply%2 ?      /*black to move*/ \
                      ((board)->bits[BP] & TRANS(0x5ull, (board)->enpas%8 - 1, 3) ? EPHASHRAW((board)->enpas) : 0ull) :\
                      ((board)->bits[WP] & TRANS(0x5ull, (board)->enpas%8 - 1, 4) ? EPHASHRAW((board)->enpas) : 0ull))
                      
//note this only works when given a single castling flag, not a set of flags
#define CASTLEHASH(flag) (hashvals[768 + (bsf(flag))])
#define WHITETURNHASH hashvals[780]


#endif
