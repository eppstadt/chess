#include <stdint.h>
#include <stdio.h>

uint64_t inWhiteCheck;
uint64_t inBlackCheck;
uint64_t tilesWithPieces;
uint64_t tilesWithWhitePieces;
uint64_t tilesWithBlackPieces;

unsigned char board[64];

uint64_t tilesWithWhiteRooks;
uint64_t tilesWithWhitePawns;
uint64_t tilesWithWhiteKnights;
uint64_t tilesWithWhiteBishops;
uint64_t tilesWithWhiteQueens;
uint64_t tilesWithWhiteKings;

uint64_t tilesWithBlackRooks;
uint64_t tilesWithBlackPawns;
uint64_t tilesWithBlackKnights;
uint64_t tilesWithBlackBishops;
uint64_t tilesWithBlackQueens;
uint64_t tilesWithBlackKings;

uint64_t tilesWithRooks() {return tilesWithBlackRooks | tilesWithWhiteRooks;}
uint64_t tilesWithPawns() {return tilesWithBlackPawns | tilesWithWhitePawns;}
uint64_t tilesWithKnights() {return tilesWithBlackKnights | tilesWithWhiteKnights;}
uint64_t tilesWithBishops() {return tilesWithBlackBishops | tilesWithWhiteBishops;}
uint64_t tilesWithQueens() {return tilesWithBlackQueens | tilesWithWhiteQueens;}
uint64_t tilesWithKings() {return tilesWithBlackKings | tilesWithWhiteKings;}

uint64_t tilesWithSameColoredPieces;
uint64_t tilesWithOppositeColoredPieces;

/**
 * @brief Active color: WHITE means that White is to move; BLACK means that Black is to move.
*/
unsigned char playerToMove;

/**
 * @brief Updates tilesWithSameColoredPieces and tilesWithOppositeColoredPieces according to playerToMove
*/
 void updateTilesWithSameColoredPieces() {
  if(playerToMove == 8) {
    tilesWithSameColoredPieces = tilesWithWhitePieces;
    tilesWithOppositeColoredPieces = tilesWithBlackPieces;
  }
  else {
    tilesWithSameColoredPieces = tilesWithBlackPieces;
    tilesWithOppositeColoredPieces = tilesWithWhitePieces;
  }
}

#define hFile 0b0000000100000001000000010000000100000001000000010000000100000001ULL
const uint64_t gFile = (hFile << 1);
const uint64_t fFile = (hFile << 2);
const uint64_t eFile = (hFile << 3);
const uint64_t dFile = (hFile << 4);
const uint64_t cFile = (hFile << 5);
const uint64_t bFile = (hFile << 6);
const uint64_t aFile = (hFile << 7);

#define firstRank 0b0000000000000000000000000000000000000000000000000000000011111111ULL
const uint64_t secondRank = (firstRank) << 8;
const uint64_t thirdRank = (firstRank) << 16;
const uint64_t fourthRank = (firstRank) << 24;
const uint64_t fifthRank = (firstRank) << 32;
const uint64_t sixthRank = (firstRank) << 40;
const uint64_t seventhRank = (firstRank) << 48;
const uint64_t eigthRank = (firstRank) << 56;

typedef enum {false, true} bool;

// The castling ability is represented as an array of 4 booleans, where each index corresponds to a specific castling option:
// 0: Black kingside, 1: Black queenside, 2: White kingside, 3: White queenside
bool castlingAbility[4];

//En passant target square: This is a square over which a pawn has just passed while moving two squares; it is given as the position of the tile from 0 to 63. If there is no en passant target square, the value is -1. This is recorded regardless of whether there is a pawn in position to capture en passant.
int epsquare;

//Halfmove clock: The number of halfmoves since the last capture or pawn advance, used for the fifty-move rule.
int numOfHalveMoves;

//Fullmove number: The number of the full moves. It starts at 1 and is incremented after Black's move.
int numOfFullMoves;

//Pieces: PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6, WHITE = 8, BLACK = 16
typedef int Piece;
#define PAWN   1
#define KNIGHT 2
#define BISHOP 3
#define ROOK   4
#define QUEEN  5
#define KING   6

#define WHITE  8
#define BLACK  16

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

        if(row < 8 && col < 8) {
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
        } else {
            printf("Invalid FEN: %s\n", FENPosition);
            return;
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

uint64_t getPossibleMovesPawn(uint64_t startingPos) {
  uint64_t possibleMoves = 0;

  if(playerToMove == WHITE && isNotOn(startingPos, eigthRank)) {
    if(isNotOn(startingPos, hFile) && isOn(startingPos << 7, tilesWithOppositeColoredPieces | (((uint64_t)1) << epsquare))) {
      possibleMoves |= startingPos << 7;
    }
    if(isNotOn(startingPos, aFile) && isOn(startingPos << 9, tilesWithOppositeColoredPieces | (((uint64_t)1) << epsquare))) {
      possibleMoves |= startingPos << 9;
    }
    if(isNotOn(startingPos << 8, tilesWithPieces)) {
      possibleMoves |= startingPos << 8;
      if(isOn(startingPos, secondRank) && isNotOn(startingPos << 16, tilesWithPieces)) {
        possibleMoves |= startingPos << 16;
      }
    }
  } else if(playerToMove == BLACK && isNotOn(startingPos, firstRank)) {
    if(isNotOn(startingPos, hFile) && isOn(startingPos >> 9, tilesWithOppositeColoredPieces | (((uint64_t)1) << epsquare))) {
      possibleMoves |= startingPos >> 9;
    }
    if(isNotOn(startingPos, aFile) && isOn(startingPos >> 7, tilesWithOppositeColoredPieces | (((uint64_t)1) << epsquare))) {
      possibleMoves |= startingPos >> 7;
    }
    if(isNotOn(startingPos >> 8, tilesWithPieces)) {
      possibleMoves |= startingPos >> 8;
      if(isOn(startingPos, seventhRank) && isNotOn(startingPos >> 16, tilesWithPieces)) {
        possibleMoves |= startingPos >> 16;
      }
    }
  }

  return possibleMoves;
}

uint64_t getPossibleMovesKnight(uint64_t startingPos) {
  uint64_t possibleMoves = 0;

  if(isNotOn(startingPos, aFile)) {
    possibleMoves |= startingPos << 17;
    possibleMoves |= startingPos >> 15;
  }

  if(isNotOn(startingPos, (aFile | bFile))) {
    possibleMoves |= startingPos >> 6;
    possibleMoves |= startingPos << 10;
  }

  if(isNotOn(startingPos, (hFile | gFile))) {
    possibleMoves |= startingPos << 6;
    possibleMoves |= startingPos >> 10;
  }

  if(isNotOn(startingPos, hFile)) {
    possibleMoves |= startingPos << 15;
    possibleMoves |= startingPos >> 17;
  }

  return possibleMoves;
}

uint64_t getPossibleMovesBishop(uint64_t startingPos) { 
  uint64_t possibleMoves = 0;

  uint64_t pointer = startingPos;

  // Diagonal up-right
  while (isNotOn(pointer, hFile | eigthRank) && isNotOn(pointer, tilesWithSameColoredPieces)) {
      pointer <<= 7;
      possibleMoves |= pointer;
      if (isOn(pointer, tilesWithOppositeColoredPieces)) {
          break;
      }
  }

  pointer = startingPos;
  // Diagonal up-left
  while (isNotOn(pointer, aFile | eigthRank) && isNotOn(pointer, tilesWithSameColoredPieces)) {
    pointer <<= 9;
    possibleMoves |= pointer;
    if (isOn(pointer, tilesWithOppositeColoredPieces)) {
      break;
    }
  }

  pointer = startingPos;
  // Diagonal down-right
  while (isNotOn(pointer, hFile | firstRank) && isNotOn(pointer, tilesWithSameColoredPieces)) {
    pointer >>= 9;
    possibleMoves |= pointer;
    if (isOn(pointer, tilesWithOppositeColoredPieces)) {
      break;
    }
  }

  pointer = startingPos;
  // Diagonal down-left
  while (isNotOn(pointer, aFile | firstRank) && isNotOn(pointer, tilesWithSameColoredPieces)) {
    pointer >>= 7;
    possibleMoves |= pointer;
    if (isOn(pointer, tilesWithOppositeColoredPieces)) {
      break;
    }
  }

  return possibleMoves;
}

uint64_t getPossibleMovesRook(uint64_t startingPos) { 
  uint64_t possibleMoves = 0;

  uint64_t pointer = startingPos;
  while(isNotOn(pointer, hFile) && isNotOn(pointer, tilesWithSameColoredPieces)) {
    pointer >>= 1;
    possibleMoves |= pointer;
    if(isOn(pointer, tilesWithOppositeColoredPieces)) {
      break;
    }
  }

  pointer = startingPos;
  while(isNotOn(pointer, aFile)) {
    pointer <<= 1;
    possibleMoves |= pointer;
    if(isOn(pointer, tilesWithOppositeColoredPieces)) {
      break;
    }
  }

  pointer = startingPos;
  while(isNotOn(pointer, firstRank)) {
    pointer >>= 8;
    possibleMoves |= pointer;
    if(isOn(pointer, tilesWithOppositeColoredPieces)) {
      break;
    }
  }

  pointer = startingPos;
  while(isNotOn(pointer, eigthRank)) {
    pointer <<= 8;
    possibleMoves |= pointer;
    if(isOn(pointer, tilesWithOppositeColoredPieces)) {
      break;
    }
  }

  return possibleMoves;
 }

uint64_t getPossibleMovesQueen(uint64_t startingPos) {
  return getPossibleMovesBishop(startingPos) | getPossibleMovesRook(startingPos);
}

uint64_t getPossibleMovesKing(uint64_t startingPos) {
  uint64_t possibleMoves = 0;

  possibleMoves |= startingPos << 8;
  possibleMoves |= startingPos >> 8;

  if(isNotOn(startingPos, aFile)) {
    possibleMoves |= startingPos << 1;
    possibleMoves |= startingPos << 9;
    possibleMoves |= startingPos >> 7;
  }

  if(isNotOn(startingPos, hFile)) {
    possibleMoves |= startingPos << 7;
    possibleMoves |= startingPos >> 1;
    possibleMoves |= startingPos >> 9;
  }

  if((castlingAbility[0] || castlingAbility[2]) && isNotOn(startingPos >> 1, tilesWithPieces) && isNotOn(startingPos >> 2, tilesWithPieces)) {
    possibleMoves |= startingPos >> 2; // kingside castling
  }

  if((castlingAbility[1] || castlingAbility[3]) && isNotOn(startingPos << 1, tilesWithPieces) && isNotOn(startingPos << 2, tilesWithPieces)) {
    possibleMoves |= startingPos << 2; // queenside castling
  }

  return possibleMoves;
}

uint64_t getPossibleMoves(int position) {

  uint64_t possibleMoves = 0;
  uint64_t startingPos = ((uint64_t)1) << position;

  switch(board[position]) {
    case PAWN | WHITE:
      possibleMoves = getPossibleMovesPawn(startingPos);
      break;
    case PAWN | BLACK:
      possibleMoves = getPossibleMovesPawn(startingPos);
      break;
    case KNIGHT | WHITE:
      possibleMoves = getPossibleMovesKnight(startingPos);
      break;
    case KNIGHT | BLACK:
      possibleMoves = getPossibleMovesKnight(startingPos);
      break;
    case BISHOP | WHITE:
      possibleMoves = getPossibleMovesBishop(startingPos);
      break;
    case BISHOP | BLACK:
      possibleMoves = getPossibleMovesBishop(startingPos);
      break;
    case ROOK | WHITE:
      possibleMoves = getPossibleMovesRook(startingPos);
      break;
    case ROOK | BLACK:
      possibleMoves = getPossibleMovesRook(startingPos);
      break;
    case QUEEN | WHITE:
      possibleMoves = getPossibleMovesQueen(startingPos);
      break;
    case QUEEN | BLACK:
      possibleMoves = getPossibleMovesQueen(startingPos);
      break;
    case KING | WHITE:
      possibleMoves = getPossibleMovesKing(startingPos);
      break;
    case KING | BLACK:
      possibleMoves = getPossibleMovesKing(startingPos);
      break;
  }

  return possibleMoves;
}

bool isNotOn(uint64_t pos, uint64_t bitboard) {
  return (pos & bitboard) == 0;
}

bool isOn(uint64_t pos, uint64_t bitboard) {
  return (pos & bitboard) != 0;
}

int main() {
    uint64_t startingPos = ((uint64_t)1) << 15; // Example position for a piece, e.g., a knight on b3

    loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    printBoard();
    printBitmask(getPossibleMovesQueen(startingPos));
}