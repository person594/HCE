int fromAlg(Position *pos, char* str);
void toAlg(Position *pos, int move, char* str);

int getPiece(char);

//polls player for a move, and returns the resulting move.  Also accepts and responds to non-move commands and input, but only returns upon receiving a valid move.
int getInputMove(Position *pos);
