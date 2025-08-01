#ifndef MAIN_H
#define MAIN_H

#include "export.h"

//types: PAWN = 1, KNIGHT = 2, BISHOP = 4, ROOK = 8, QUEEN = 16, KING = 32, WHITE = 64, BLACK = 128
typedef unsigned int PieceType;
#define PAWN   1
#define KNIGHT 2
#define BISHOP 4
#define ROOK   8
#define QUEEN  16
#define KING   32

#define WHITE  64
#define BLACK  128

EXPORT void innit(char* FENPosition);
EXPORT char* getLongAlgebraicNotationFromPosition(short position);
EXPORT int getPositionFromLongAlgebraicNotation(char* notation);
EXPORT unsigned short* getPossibleMoves();
EXPORT void doMove(int position);
EXPORT void doLongAlgebraicNotationMove(char* notation);
EXPORT int getCheckMate();
EXPORT int getStaleMate();
EXPORT PieceType *getBoard();
EXPORT char getPlayerToMove();

#endif // MAIN_H