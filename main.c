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

#define hRow 0b0000000100000001000000010000000100000001000000010000000100000001
#define gRow hRow << 1
#define fRow hRow << 2
#define eRow hRow << 3
#define dRow hRow << 4
#define cRow hRow << 5
#define bRow hRow << 6
#define aRow hRow << 7

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
  for (int i = 64; i >= 1; i--) {
    int t = (mask & (((uint64_t) 1) << (i-1))) == 0? 0 : 1;
    printf("%d ", t);
    if ((i-1) % 8 == 0)
      printf("\n");
  }

  printf("\n");
}

uint64_t getPossibleMoves(int position) {

  uint64_t possibleMoves;
  uint64_t startingPos = ((uint64_t)1) << position;

  //get Color of piece and change this below with color
  possibleMoves &= !tilesWithPieces;

  return possibleMoves;
}

uint64_t getPossibleMovesPawn(uint64_t startingPos) {

  return 0;
}

uint64_t getPossibleMovesKnight(uint64_t startingPos) {
  uint64_t possibleMoves = 0;

  if((startingPos & aRow) == 0) {
    possibleMoves |= startingPos << 17;
    possibleMoves |= startingPos >> 15;
  }

  if((startingPos & (aRow | bRow)) == 0) {
    possibleMoves |= startingPos >> 6;
    possibleMoves |= startingPos << 10;
  }

  if((startingPos & (hRow | gRow)) == 0) {
    possibleMoves |= startingPos << 6;
    possibleMoves |= startingPos >> 10;
  }

  if((startingPos & hRow) == 0) {
    possibleMoves |= startingPos << 15;
    possibleMoves |= startingPos >> 17;
  }

  return possibleMoves;
}

uint64_t getPossibleMovesBishop(uint64_t startingPos) { return 0; }

uint64_t getPossibleMovesRook(uint64_t startingPos) { return 0; }

uint64_t getPossibleMovesQueen(uint64_t startingPos) {
  return getPossibleMovesBishop(startingPos) | getPossibleMovesRook(startingPos);
}

uint64_t getPossibleMovesKing(uint64_t startingPos) {
  uint64_t possibleMoves = 0;

  possibleMoves |= startingPos << 8;
  possibleMoves |= startingPos >> 8;

  if((startingPos & aRow) == 0) {
    possibleMoves |= startingPos << 1;
    possibleMoves |= startingPos << 9;
    possibleMoves |= startingPos >> 7;
  }

  if((startingPos & hRow) == 0) {
    possibleMoves |= startingPos << 7;
    possibleMoves |= startingPos >> 1;
    possibleMoves |= startingPos >> 9;
  }

  return possibleMoves;
}

int main() {
    uint64_t startingPos = ((uint64_t)1) << 7;

    loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    printBoard();
    printBitmask(getPossibleMovesKing(startingPos));
}