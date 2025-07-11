#ifndef MAIN_H
#define MAIN_H

#include "export.h"

EXPORT void innit();
EXPORT char* getLongAlgebraicNotationFromPosition(short position);
EXPORT int getPositionFromLongAlgebraicNotation(char* notation);
EXPORT unsigned short* getPossibleMoves();
EXPORT void doMove(int position, int promotionType);
EXPORT void doLongAlgebraicNotationMove(char* notation);
EXPORT int isCheckMate();

#endif // MAIN_H