#include <stdint.h>
#include <stdio.h>

uint64_t inWhiteCheck;
uint64_t inBlackCheck;
uint64_t tilesWithPieces;

unsigned char board[64];

uint64_t tilesWithRooks;
uint64_t tilesWithPawns;
uint64_t tilesWithKnights;
uint64_t tilesWithBishops;
uint64_t tilesWithQueens;
uint64_t tilesWithKings;

unsigned char playerToMove;
int castlingAbility[4];
int epsquare;

int numOfHalveMoves;
int numOfFullMoves;

typedef int Piece;
    const Piece PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6,
    
    WHITE = 8,
    BLACK = 16;

void loadFEN(char *FENPosition) {

    char *position = FENPosition;

    int row = 0;
    int col = 0;
    char color = WHITE;
    char piece;

    for(; *position != ' '; position++) {

        if(*position == '/') {
            row++;
            col = 0;
            continue;
        }
        
        if(*position > 48 && *position < 57) {
            col += *position - 48;
            continue;
        }

        if(*position > 'Z') {
            color = BLACK;
            piece = *position;
        } else {
            color = WHITE;
            piece = *position + 32;
        }

        switch(piece) {
            case 'r':
                board[8*row + col] = ROOK | color;
                break;
            case 'n':
                board[8*row + col] = KNIGHT | color;
                break;
            case 'b':
                board[8*row + col] = BISHOP | color;
                break;
            case 'q':
                board[8*row + col] = QUEEN | color;
                break;
            case 'k':
                board[8*row + col] = KING | color;
                break;
            case 'p':
                board[8*row + col] = PAWN | color;
                break;
        }

        col++;
    }

    position++;
    playerToMove = *position == 'w'? WHITE : BLACK;
    position += 2;

    for (; *position != ' '; position++) {
        switch (*position) {
            case '-':
                break;
            case 'k':
                castlingAbility[0] = 1;
                break;
            case 'q':
                castlingAbility[1] = 1;
                break;
            case 'K':
                castlingAbility[2] = 1;
                break;
            case 'Q':
                castlingAbility[3] = 1;
                break;
        }
    }

    position++;
    if(*position != '-') {
        epsquare = *position - 97 + 8 * (*(position+1) - 48);
    }

    position += 2;

    sscanf(position, "%d %d", &numOfHalveMoves, &numOfFullMoves);
}

void printBoard() {
    for(int i = 1; i < 65; i++) {
        printf("%d ", board[i-1]);
        if(i % 8 == 0) printf("\n");
    }
    printf("\n");
}

void printBitmask(uint64_t mask) {
  for (int i = 1; i < 65; i++) {
    int t = (mask & (((uint64_t) 1) << (i-1))) == 0? 0 : 1;
    printf("%d ", t);
    if (i % 8 == 0)
      printf("\n");
  }

  printf("\n");
}

uint64_t getPossibleMoves(int position) {

  uint64_t possibleMoves;

  //get Color of piece and change this below with color
  possibleMoves &= !tilesWithPieces;

  return possibleMoves;
}

uint64_t getPossibleMovesPawn(int position) {

  return 0;
}

uint64_t getPossibleMovesKnight(int position) {

  return 0;
}

uint64_t getPossibleMovesBishop(int position) { return 0; }

uint64_t getPossibleMovesRook(int position) { return 0; }

uint64_t getPossibleMovesQueen(int position) {
  return getPossibleMovesBishop(position) | getPossibleMovesRook(position);
}

uint64_t getPossibleMovesKing(int position) {
  uint64_t startingPos = ((uint64_t)1) << position;
  uint64_t possibleMoves;

  possibleMoves |= startingPos << 1;
  possibleMoves |= startingPos << 7;
  possibleMoves |= startingPos << 8;
  possibleMoves |= startingPos << 9;

  possibleMoves |= startingPos >> 1;
  possibleMoves |= startingPos >> 7;
  possibleMoves |= startingPos >> 8;
  possibleMoves |= startingPos >> 9;

  return possibleMoves;
}

int main() {
    loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    printBoard();
    printBitmask(getPossibleMovesKing(2));
}