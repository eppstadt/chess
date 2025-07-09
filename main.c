#include <stdint.h>
#include <stdio.h>
#include <string.h>

//types: PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6, WHITE = 8, BLACK = 16
typedef unsigned int PieceType;
#define PAWN   1
#define KNIGHT 2
#define BISHOP 3
#define ROOK   4
#define QUEEN  5
#define KING   6

#define WHITE  8
#define BLACK  16

typedef struct {
  PieceType pieceType;
  uint64_t attackingBitBoard;
  uint64_t movingBitBoard;
} Piece;

// Just a helper struct to return more than one value from a function
typedef struct {
  uint64_t x;
  uint64_t y;
} Point;

Piece board[64];

uint64_t inWhiteCheck;
uint64_t inBlackCheck;
uint64_t tilesWithWhitePieces;
uint64_t tilesWithBlackPieces;
uint64_t tilesWithPieces() {return tilesWithWhitePieces | tilesWithBlackPieces;}

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
  if(playerToMove == WHITE) {
    tilesWithSameColoredPieces = tilesWithWhitePieces;
    tilesWithOppositeColoredPieces = tilesWithBlackPieces;
  }
  else {
    tilesWithSameColoredPieces = tilesWithBlackPieces;
    tilesWithOppositeColoredPieces = tilesWithWhitePieces;
  }
}

/**
 * @brief Bitboard representation of the chess board files and ranks.
 * 
 * The bitboards are defined as 64-bit unsigned integers, where each bit represents a square on the chess board.
 * The least significant bit (LSB) corresponds to the a1 square, and the most significant bit (MSB) corresponds to the h8 square.
 */
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

//Stores the last Move done. OldPos = [0], newPos = [1]
int lastMove[2];

//Stores all the possible Moves in the current position
//FORMAT  1bit Queen Promotion, 1bit KnightPromotion, 1bit RookPromotion, 1bit BishopPromotion, 6bits for newPos, 6bits for oldPos
unsigned short possibleMoves[218];

/**
 * @brief Loads the position from the given FEN-String into the programm
 * 
 * @param FENPosition The FEN-String of a position
 * 
 * This Function iterates over the FEN-String loading it into the programm
 */
void loadFEN(char *FENPosition) {

    char *position = FENPosition;

    int row = 7;
    int col = 7;
    char color = BLACK;
    char piece;

    for(; *position != ' '; position++) {

        if(*position == '/') {
            row--;
            col = 7;
            continue;
        }
        
        if(*position >= '1' && *position <= '8') {
            col -= *position - 48;
            continue;
        }

        if(*position > 'Z') {
            color = BLACK;
            piece = *position;
        } else {
            color = WHITE;
            piece = *position + 32;
        }

        if(row >= 0 && col >= 0) {
          switch(piece) {
              case 'r':
                  board[8*row + col].pieceType = ROOK | color;
                  break;
              case 'n':
                  board[8*row + col].pieceType = KNIGHT | color;
                  break;
              case 'b':
                  board[8*row + col].pieceType = BISHOP | color;
                  break;
              case 'q':
                  board[8*row + col].pieceType = QUEEN | color;
                  break;
              case 'k':
                  board[8*row + col].pieceType = KING | color;
                  break;
              case 'p':
                  board[8*row + col].pieceType = PAWN | color;
                  break;
          }
        } else {
            printf("Invalid FEN: %s\n", FENPosition);
            return;
        }
        col--;
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

/**
 * @brief Initializes all the tilesWithPieces Variables.
 * 
 * This function iterates through the board array and sets the tilesWithWhitePieces and tilesWithBlackPieces bitboards
 * aswell as the tilesWithSameColoredPieces and tilesWithOppositeColoredPieces bitboards.
 * 
 * @note Requires the board array to be initialized with piece types before calling this function.
 */
void innitTilesWithPieces() {
    tilesWithWhitePieces = 0;
    tilesWithBlackPieces = 0;

    for(int i = 0; i < 64; i++) {
        if(board[i].pieceType == 0) continue;
        if(board[i].pieceType & WHITE) {
            tilesWithWhitePieces |= ((uint64_t)1) << i;
        } else {
            tilesWithBlackPieces |= ((uint64_t)1) << i;
        }
    }

    updateTilesWithSameColoredPieces();
}

/**
 * @brief Prints the current state of the chess board.
 * 
 * This function iterates through the board array and prints the piece type at each position.
 * It formats the output to display the board in an 8x8 grid.
 */
void printBoard() {
    for(int i = 63; i >= 0; i--) {
        switch (board[i].pieceType) {
          case PAWN | WHITE:
            printf("P ");
            break;
          case KNIGHT | WHITE:
            printf("N ");
            break;
          case BISHOP | WHITE:
            printf("B ");
            break;
          case ROOK | WHITE:
            printf("R ");
            break;
          case QUEEN | WHITE:
            printf("Q ");
            break;
          case KING | WHITE:
            printf("K ");
            break;
          case KING | BLACK:
            printf("k ");
            break;
          case QUEEN | BLACK:
            printf("q "); 
            break;
          case ROOK | BLACK:
            printf("r ");
            break;
          case BISHOP | BLACK:
            printf("b ");
            break;
          case KNIGHT | BLACK:
            printf("n ");
            break;
          case PAWN | BLACK:
            printf("p ");
            break;
          default:
            printf(". ");
            break;
        }
        if(i % 8 == 0) printf("\n");
    }
    printf("\n");
}

/**
 * @brief Prints a bitmask representing the bits of a 64-bit integer.
 * 
 * This function prints the bits of a 64-bit integer in a formatted manner,
 * displaying each bit as either 0 or 1, with a newline after every 8 bits.
 * 
 * @param mask The 64-bit integer to be printed as a bitmask.
 */
void printBitmask(uint64_t mask) {
  for (int i = 64; i >= 1; i--) {
    int t = (mask & (((uint64_t) 1) << (i-1))) == 0? 0 : 1;
    printf("%d ", t);
    if ((i-1) % 8 == 0)
      printf("\n");
  }

  printf("\n");
}

/**
 * @brief Checks if a position is not occupied by a piece on the given bitboard.
 * 
 * This function checks if a specific position (bit) is not set in the provided bitboard.
 * 
 * @param pos The position to check (0-63).
 * @param bitboard The bitboard to check against.
 * @return bool Returns true if the position is not occupied, false otherwise.
 */
bool isNotOn(uint64_t pos, uint64_t bitboard) {
  return (pos & bitboard) == 0;
}

/**
 * @brief Checks if a position is occupied by a piece on the given bitboard.
 * 
 * This function checks if a specific position (bit) is set in the provided bitboard.
 * 
 * @param pos The position to check (0-63).
 * @param bitboard The bitboard to check against.
 * @return bool Returns true if the position is occupied, false otherwise.
 */
bool isOn(uint64_t pos, uint64_t bitboard) {
  return (pos & bitboard) != 0;
}

/**
 * @brief Generates a bitboard representing the possible moves for a pawn at a given position.
 * 
 * This function calculates the possible moves for a pawn based on its current position, taking into account
 * the edges of the board and the presence of other pieces and the option to en passant. It returns a bitboard representing the squares
 * where the pawn can move.
 * 
 * @param startingPos The position of the pawn on the board (0-63).
 * @return uint64_t A bitboard representing the possible moves for the pawn.
 */
Point getPossibleMoveBitBoardPawn(uint64_t startingPos) {
  uint64_t possibleAttacks = 0;
  uint64_t movingBitBoard = 0;

  if(playerToMove == WHITE && isNotOn(startingPos, eigthRank)) {
    if(isNotOn(startingPos, hFile) && isOn(startingPos << 7, tilesWithOppositeColoredPieces | (((uint64_t)1) << epsquare))) {
      possibleAttacks |= startingPos << 7;
    }
    if(isNotOn(startingPos, aFile) && isOn(startingPos << 9, tilesWithOppositeColoredPieces | (((uint64_t)1) << epsquare))) {
      possibleAttacks |= startingPos << 9;
    }
    if(isNotOn(startingPos << 8, tilesWithPieces())) {
      movingBitBoard |= startingPos << 8;
      if(isOn(startingPos, secondRank) && isNotOn(startingPos << 16, tilesWithPieces())) {
        movingBitBoard |= startingPos << 16;
      }
    }
  } else if(playerToMove == BLACK && isNotOn(startingPos, firstRank)) {
    if(isNotOn(startingPos, hFile) && isOn(startingPos >> 9, tilesWithOppositeColoredPieces | (((uint64_t)1) << epsquare))) {
      possibleAttacks |= startingPos >> 9;
    }
    if(isNotOn(startingPos, aFile) && isOn(startingPos >> 7, tilesWithOppositeColoredPieces | (((uint64_t)1) << epsquare))) {
      possibleAttacks |= startingPos >> 7;
    }
    if(isNotOn(startingPos >> 8, tilesWithPieces())) {
      movingBitBoard |= startingPos >> 8;
      if(isOn(startingPos, seventhRank) && isNotOn(startingPos >> 16, tilesWithPieces())) {
        movingBitBoard |= startingPos >> 16;
      }
    }
  }

  return (Point) {possibleAttacks, movingBitBoard};
}

/**
 * @brief Generates a bitboard representing the possible moves for a knight at a given position.
 * 
 * This function calculates the possible moves for a knight based on its current position, taking into account
 * the edges of the board. It returns a bitboard representing the squares where the knight can move.
 * 
 * @param startingPos The position of the knight on the board (0-63).
 * @return uint64_t A bitboard representing the possible moves for the knight.
 */
uint64_t getPossibleMoveBitBoardKnight(uint64_t startingPos) {
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

/**
 * @brief Generates a bitboard representing the possible moves for a bishop at a given position.
 * 
 * This function calculates the possible moves for a bishop based on its current position, taking into account
 * the edges of the board and the presence of other pieces. It returns a bitboard representing the squares
 * where the bishop can move.
 * 
 * @param startingPos The position of the bishop on the board (0-63).
 * @return uint64_t A bitboard representing the possible moves for the bishop.
 */
uint64_t getPossibleMoveBitBoardBishop(uint64_t startingPos) { 
  uint64_t possibleMoves = 0;

  uint64_t pointer = startingPos;

  // Diagonal up-right
  while (isNotOn(pointer, hFile | eigthRank)) {
      pointer <<= 7;
      possibleMoves |= pointer;
      if (isOn(pointer, tilesWithPieces())) {
          break;
      }
  }

  pointer = startingPos;
  // Diagonal up-left
  while (isNotOn(pointer, aFile | eigthRank)) {
    pointer <<= 9;
    possibleMoves |= pointer;
    if (isOn(pointer, tilesWithPieces())) {
      break;
    }
  }

  pointer = startingPos;
  // Diagonal down-right
  while (isNotOn(pointer, hFile | firstRank)) {
    pointer >>= 9;
    possibleMoves |= pointer;
    if (isOn(pointer, tilesWithPieces())) {
      break;
    }
  }

  pointer = startingPos;
  // Diagonal down-left
  while (isNotOn(pointer, aFile | firstRank)) {
    pointer >>= 7;
    possibleMoves |= pointer;
    if (isOn(pointer, tilesWithPieces())) {
      break;
    }
  }

  return possibleMoves;
}

/**
 * @brief Generates a bitboard representing the possible moves for a rook at a given position.
 * 
 * This function calculates the possible moves for a rook based on its current position, taking into account
 * the edges of the board and the presence of other pieces. It returns a bitboard representing the squares
 * where the rook can move.
 * 
 * @param startingPos The position of the rook on the board (0-63).
 * @return uint64_t A bitboard representing the possible moves for the rook.
 */
uint64_t getPossibleMoveBitBoardRook(uint64_t startingPos) { 
  uint64_t possibleMoves = 0;

  uint64_t pointer = startingPos;

  while(isNotOn(pointer, hFile)) {
    pointer >>= 1;
    possibleMoves |= pointer;
    if(isOn(pointer, tilesWithPieces())) {
      break;
    }
  }

  pointer = startingPos;
  while(isNotOn(pointer, aFile)) {
    pointer <<= 1;
    possibleMoves |= pointer;
    if(isOn(pointer, tilesWithPieces())) {
      break;
    }
  }

  pointer = startingPos;
  while(isNotOn(pointer, firstRank)) {
    pointer >>= 8;
    possibleMoves |= pointer;
    if(isOn(pointer, tilesWithPieces())) {
      break;
    }
  }

  pointer = startingPos;
  while(isNotOn(pointer, eigthRank)) {
    pointer <<= 8;
    possibleMoves |= pointer;
    if(isOn(pointer, tilesWithPieces())) {
      break;
    }
  }

  return possibleMoves;
 }

uint64_t getPossibleMoveBitBoardQueen(uint64_t startingPos) {
  return getPossibleMoveBitBoardBishop(startingPos) | getPossibleMoveBitBoardRook(startingPos);
}

/**
 * @brief Generates a bitboard representing the possible moves for a king at a given position.
 * 
 * This function calculates the possible moves for a king based on its current position, taking into account
 * the edges of the board and castling rights. It returns a bitboard representing the squares where the king can move.
 * 
 * @param startingPos The position of the king on the board (0-63).
 * @return uint64_t A bitboard representing the possible moves for the king.
 */
Point getPossibleMoveBitBoardKing(uint64_t startingPos) {
  uint64_t possibleMoves = 0;
  uint64_t movingBitBoard = 0;

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

  if((castlingAbility[0] || castlingAbility[2]) && isNotOn(startingPos >> 1, tilesWithPieces()) && isNotOn(startingPos >> 2, tilesWithPieces())) {
    movingBitBoard |= startingPos >> 2; // kingside castling
  }

  if((castlingAbility[1] || castlingAbility[3]) && isNotOn(startingPos << 1, tilesWithPieces()) && isNotOn(startingPos << 2, tilesWithPieces())) {
    movingBitBoard |= startingPos << 2; // queenside castling
  }

  return (Point) {possibleMoves, movingBitBoard};
}

/**
 * @brief Generates a bitboard representing the possible moves for a piece at a given position.
 * 
 * This function checks the type of the piece at the specified position and calls the appropriate
 * function to get the possible moves for that piece type. It returns a bitboard representing the
 * squares where the piece can move.
 * 
 * @param position The position of the piece on the board (0-63).
 * @return uint64_t A bitboard representing the possible moves for the piece.
 */
Point getPossibleMoveBitBoard(int position) {

  Point possibleMovesPoint = {0, 0};

  uint64_t possibleMoves = 0;
  uint64_t startingPos = ((uint64_t)1) << position;

  int pieceType = board[position].pieceType & 7; // Mask to get the piece type without color

  switch(pieceType) {
    case PAWN:
      possibleMovesPoint = getPossibleMoveBitBoardPawn(startingPos);
      break;
    case KNIGHT:
      possibleMoves = getPossibleMoveBitBoardKnight(startingPos);
      break;
    case BISHOP:
      possibleMoves = getPossibleMoveBitBoardBishop(startingPos);
      break;
    case ROOK:
      possibleMoves = getPossibleMoveBitBoardRook(startingPos);
      break;
    case QUEEN:
      possibleMoves = getPossibleMoveBitBoardQueen(startingPos);
      break;
    case KING:
      possibleMovesPoint = getPossibleMoveBitBoardKing(startingPos);
      break;
  }

  possibleMovesPoint.x |= possibleMoves;

  return possibleMovesPoint;
}

/**
 * @brief Generates a bitboard representing the squares where the balck king is in check.
 * 
 * This function iterates through all pieces on the board, checking if they are white pieces
 * and if they can attack the square of the black king. It updates the attackingBitBoard for
 * each white- QUEEN/ROOK/BISHOP and the current piece move and accumulates the squares where 
 * the black king is in check if updateAttackingBitBoard is set to true.
 * 
 * @return uint64_t A bitboard representing the squares where the black king is in check.
 */
uint64_t generateWhiteCheckBitBoard(bool updateAttackingBitBoard) {
  uint64_t inWhiteCheckTMP = 0;
  Point tmp;

  for(int i = 0; i < 64; i++) {
    if(board[i].pieceType == 0 || board[i].pieceType > 15) continue;
    tmp.x = board[i].attackingBitBoard;
    if(board[i].pieceType == (QUEEN | WHITE) || board[i].pieceType == (ROOK | WHITE) || board[i].pieceType == (BISHOP | WHITE) || i == lastMove[1]) {
      tmp = getPossibleMoveBitBoard(i);
      if(updateAttackingBitBoard) {
        board[i].attackingBitBoard = tmp.x;
        board[i].movingBitBoard = tmp.y;
      }
    }
    inWhiteCheckTMP |= tmp.x;
  }

  return inWhiteCheckTMP;
}

/**
 * @brief Generates a bitboard representing the squares where the black king is in check.
 * 
 * This function iterates through all pieces on the board, checking if they are black pieces
 * and if they can attack the square of the white king. It updates the attackingBitBoard for
 * each black- QUEEN/ROOK/BISHOP and the current piece move and accumulates the squares where 
 * the white king is in check if updateAttackingBitBoard is set to true.
 * 
 * @return uint64_t A bitboard representing the squares where the white king is in check.
 */
uint64_t generateBlackCheckBitBoard(bool updateAttackingBitBoard) {
  uint64_t inBlackCheckTMP = 0;
  Point tmp;

  for(int i = 0; i < 64; i++) {
    if(board[i].pieceType < 16) continue;
    tmp.x = board[i].attackingBitBoard;
    if(board[i].pieceType == (QUEEN | BLACK) || board[i].pieceType == (ROOK | BLACK) || board[i].pieceType == (BISHOP | BLACK) || i == lastMove[1]) {
      tmp = getPossibleMoveBitBoard(i);
      if(updateAttackingBitBoard) {
        board[i].attackingBitBoard = tmp.x;
        board[i].movingBitBoard = tmp.y;
      }
    }
    inBlackCheckTMP |= tmp.x;
  }

  return inBlackCheckTMP;
}

/**
 * @brief Initializes the attacking bitboards for all pieces on the board.
 * 
 * This function iterates through all pieces on the board and generates their attacking bitboards
 * using the getPossibleMoveBitBoard function. It updates the attackingBitBoard field of each piece.
 */
void innitCheckBitBoards() {
  for(int i = 0; i < 64; i++) {
    if(board[i].pieceType == 0) continue;
    Point tmp = getPossibleMoveBitBoard(i);
    board[i].attackingBitBoard = tmp.x;
    board[i].movingBitBoard = tmp.y;
  }
}

/**
 * @brief Simulates a move by updating the board and the tiles with pieces.
 * 
 * @param oldPos The position of the piece before the move.
 * @param newPos The position of the piece after the move.
 * 
 * @return bool Returns true if the move would result in the king being in check, otherwise returns false.
 * 
 * @note This function does not check for legality of the move, it only updates the board and the tiles with pieces.
 */
bool simultateMove(int oldPos, int newPos, int promotionType) {

  bool invalidMove = false;

  // Update the board
  Piece temp = board[newPos];
  if(promotionType != 0) {
    board[newPos] = (Piece){promotionType | (board[newPos].pieceType & 24), 0}; // Promote the piece
  } else {
    board[newPos] = board[oldPos]; // Move the piece
  }
  Piece oldPiece = board[oldPos];
  board[oldPos] = (Piece){0, 0};

  // Update the tiles with pieces
  if(board[newPos].pieceType & WHITE) {
    tilesWithWhitePieces &= ~(((uint64_t)1) << oldPos);
    tilesWithWhitePieces |= (((uint64_t)1) << newPos);
    if(generateBlackCheckBitBoard(false) & tilesWithWhiteKings) { invalidMove = true; };
  } else {
    tilesWithBlackPieces &= ~(((uint64_t)1) << oldPos);
    tilesWithBlackPieces |= (((uint64_t)1) << newPos);
    if(generateWhiteCheckBitBoard(false) & tilesWithBlackKings) { invalidMove = true; };
  }

  // Undo the move
  board[oldPos] = oldPiece;
  board[newPos] = temp;

  if(board[oldPos].pieceType & WHITE) {
    tilesWithWhitePieces &= ~(((uint64_t)1) << newPos);
    tilesWithWhitePieces |= (((uint64_t)1) << oldPos);
  } else {
    tilesWithBlackPieces &= ~(((uint64_t)1) << newPos);
    tilesWithBlackPieces |= (((uint64_t)1) << oldPos);
  }

  return invalidMove;
}

/**
 * @brief Checks if a move is legal by simulating the move and checking if it results in the king being in check.
 * 
 * @param oldPos The position of the piece before the move.
 * @param newPos The position of the piece after the move.
 * 
 * @return bool Returns true if the move is legal, otherwise returns false.
 */
bool moveIsLegal(int oldPos, int newPos, int promotionType) {
  // Check if the move is valid
  if(oldPos < 0 || oldPos >= 64 || newPos < 0 || newPos >= 64) return false;

  if(oldPos == newPos) return false; // No move made

  if(board[oldPos].pieceType == 0) return false; // No piece at old position

  if((board[oldPos].pieceType & 24) != playerToMove) return false; // Piece is not of the current player
  
  // Check if the piece is moving to a square occupied by a piece of the same color
  if((board[oldPos].pieceType & playerToMove) == (board[newPos].pieceType & playerToMove)) return false;

  if((board[oldPos].attackingBitBoard & (((uint64_t)1) << newPos)) == 0) return false; // The piece cannot move to the new position

  if(promotionType != 0 && ((board[oldPos].pieceType & PAWN) == 0 || (newPos < 55 && newPos > 7))) return false; // Promotion is only allowed for pawns

  // Simulate the move and check if it results in the king being in check
  return !simultateMove(oldPos, newPos, promotionType);
}

/**
 * @brief Checks if a move is legal by only simulating the move without checking the legality of the piece.
 * 
 * @param oldPos The position of the piece before the move.
 * @param newPos The position of the piece after the move.
 * @param promotionType The type of promotion (0 for no promotion, BISHOP, ROOK, KNIGHT, or QUEEN).
 * 
 * @return bool Returns true if the move is legal, otherwise returns false.
 * 
 * @note This function is unsafe as it does not check if the piece is of the correct type or if the move is valid.
 * It is intended for use in scenarios where the legality of the move (besides Checks) has already been checked.
 */
bool moveIsLegalUnsafe(int oldPos, int newPos, int promotionType) {

  if((board[oldPos].pieceType & playerToMove) == (board[newPos].pieceType & playerToMove)) return false;

  return !simultateMove(oldPos, newPos, promotionType);
}

// TODO: change the comment below

/**
 * @brief Counts the number of trailing zeros in a 64-bit integer.
 *
 * This function uses the _BitScanForward64 intrinsic to find the index of the
 * least significant bit that is set. If the input is zero, it returns -1 to
 * indicate an undefined result.
 *
 * @param mask The 64-bit integer to check.
 * @return int The index of the least significant bit that is set, or -1 if the
 * input is zero.
 * @note This function is specific to Windows x64 and uses the _BitScanForward64
 * intrinsic.
 */
int countTrailingZeros(uint64_t mask) {

  if (mask == 0)
    return -1;

#ifdef _MSC_VER
  unsigned long index;
  _BitScanForward64(&index, mask) return (int)index;
#else
  return __builtin_ctzll(mask);
#endif
}

/**
 * @brief Calculates all legal moves for the current position and stores them in the possibleMoves array.
 * 
 * This function iterates through all squares on the board, checks if there is a piece of the current player,
 * and generates a bitboard of possible moves for that piece. It then checks if each move is legal and stores
 * the legal moves in the possibleMoves array.
 */
void calcAllLegalMoves() {
  int moveIndex = 0;

  /* Debugging: print the current state of the board
  printf("Calculating all legal moves for player %s...\n", playerToMove == WHITE ? "White" : "Black");
  printBitmask(tilesWithSameColoredPieces); // Debugging: print the tiles with same colored pieces
  printBitmask(tilesWithOppositeColoredPieces); // Debugging: print the tiles with opposite colored pieces
  */

  for(int i = 0; i < 64; i++) {
    if(board[i].pieceType == 0 || (board[i].pieceType & playerToMove) == 0) continue; // No piece of the right color at this position

    uint64_t possibleMoveBitBoard = board[i].attackingBitBoard;

    /* Debugging: print the piece type and position
    printf("Possible moves for piece(%d) at position %d:\n", board[i].pieceType, i);
    printBitmask(possibleMoveBitBoard); // Debugging: print the possible moves bitboard
    */

    while(possibleMoveBitBoard) {
      int movePos = countTrailingZeros(possibleMoveBitBoard); // Get the index of the least significant bit
      if(movePos == -1) break; // No more moves available
      possibleMoveBitBoard &= ~(1ULL << movePos); // Clear the bit at movePos

      if(board[movePos].pieceType & PAWN) {
        // Check for promotion
        if((playerToMove == WHITE && movePos >= 56) || (playerToMove == BLACK && movePos <= 7)) {
          // Check for promotion to Bishop, Rook, Knight or Queen
          if(moveIsLegalUnsafe(i, movePos, BISHOP)) {
            possibleMoves[moveIndex++] = i | (movePos << 6) | 0x100; // Bishop promotion
          } else if (moveIsLegalUnsafe(i, movePos, ROOK)) {
            possibleMoves[moveIndex++] = i | (movePos << 6) | 0x200; // Rook promotion
          } else if (moveIsLegalUnsafe(i, movePos, KNIGHT)) {
            possibleMoves[moveIndex++] = i | (movePos << 6) | 0x400; // Knight promotion
          } else if (moveIsLegalUnsafe(i, movePos, QUEEN)) {
            possibleMoves[moveIndex++] = i | (movePos << 6) | 0x800; // Queen promotion
          }
          continue;
        }
      }

      // Check if the move is legal
      if(moveIsLegalUnsafe(i, movePos, 0)) {
        possibleMoves[moveIndex++] = i | (movePos << 6); // Store the move in the possibleMoves array
      }
    }
  }

  possibleMoves[moveIndex] = 0; // Mark the end of the moves
}

char* getLongAlgebraicNotationFromPosition(short position) {
  static char notationBuffer[6];

  int oldPos = position & 63; // Mask to get the position in the range 0-63
  int newPos = (position >> 6) & 63; // Extract the old position from the higher bits

  char promotingPiece = '\0';
  if (position & 0x1000) {
    promotingPiece = 'b'; // Promotion to Bishop
  } else if (position & 0x2000) {
    promotingPiece = 'r'; // Promotion to Rook
  } else if (position & 0x4000) {
    promotingPiece = 'k'; // Promotion to Knight
  } else if (position & 0x8000) {
    promotingPiece = 'q'; // Promotion to Queen
  }
  

  char* notation = notationBuffer;
  notation[0] = 'h' - (oldPos % 8); // file of old position
  notation[1] = '1' + (oldPos / 8); // rank of old position
  notation[2] = 'h' - (newPos % 8); // file of new position
  notation[3] = '1' + (newPos / 8); // rank of new position
  notation[4] = promotingPiece; // Promotion indicator
  notation[5] = '\0';
  
  return notation;
}

int getPositionFromLongAlgebraicNotation(char* notation) {
  int oldPos = (8 - (notation[1] - '1')) * 8 + ('h' - notation[0]);
  int newPos = (8 - (notation[3] - '1')) * 8 + ('h' - notation[2]);

  int position = (oldPos << 6) | newPos; // Combine old and new positions

  if(notation[4] == 'b') {
    position |= 0x1000; // Bishop promotion
  } else if(notation[4] == 'r') {
    position |= 0x2000; // Rook promotion
  } else if(notation[4] == 'k') {
    position |= 0x4000; // Knight promotion
  } else if(notation[4] == 'q') {
    position |= 0x8000; // Queen promotion
  }

  return position;
}

void printPossibleMoves() {
  for(int i = 0; i < 218; i++) {
    if(possibleMoves[i] == 0) break; // End of moves
    printf("%d: %s\n", i, getLongAlgebraicNotationFromPosition(possibleMoves[i]));
  }
}

void doMove(int position, int promotionType) {
  int oldPos = (position >> 8) & 63; // Extract the old position from the higher bits
  int newPos = position & 63; // Mask to get the new position in the range 0-63

  if(moveIsLegal(oldPos, newPos, promotionType)) {

    // Update halfmove clock
    if(isOn(oldPos, tilesWithPawns()) || isOn(newPos, tilesWithPieces())) {
      numOfHalveMoves = 0; // Reset halfmove clock if a pawn moved or a piece was captured
    } else {
      numOfHalveMoves++; // Increment halfmove clock
    }

    // Update the fullmove number
    if(playerToMove == BLACK) {
      numOfFullMoves++;
    }

    board[newPos] = board[oldPos];
    board[oldPos] = (Piece){0, 0};

    lastMove[0] = oldPos;
    lastMove[1] = newPos;

    if(board[newPos].pieceType & WHITE) {
      tilesWithWhitePieces &= ~(((uint64_t)1) << oldPos);
      tilesWithWhitePieces |= (((uint64_t)1) << newPos);
      inBlackCheck = generateWhiteCheckBitBoard(true);
    } else {
      tilesWithBlackPieces &= ~(((uint64_t)1) << oldPos);
      tilesWithBlackPieces |= (((uint64_t)1) << newPos);
      inWhiteCheck = generateBlackCheckBitBoard(true);
    }

    // Update the castling rights if the king or rook has moved
    if(castlingAbility[0] || castlingAbility[1] || castlingAbility[2] || castlingAbility[3]) {
      if(oldPos == 3 || newPos == 3) { // White king moved
        castlingAbility[2] = false;
        castlingAbility[3] = false;
      } else if(oldPos == 59 || newPos == 59) { // Black king moved
        castlingAbility[0] = false;
        castlingAbility[1] = false;
      } else if(oldPos == 63 || newPos == 63) {
        castlingAbility[1] = false; // Black queenside rook moved
      } else if(oldPos == 56 || newPos == 56) {
        castlingAbility[0] = false; // Black kingside rook moved
      } else if(oldPos == 7 || newPos == 7) {
        castlingAbility[3] = false; // White queenside rook moved
      } else if(oldPos == 0 || newPos == 0) {
        castlingAbility[2] = false; // White kingside rook moved
      }
    }

    // Update the en passant square if a pawn moved two squares forward
    if(isOn(oldPos, tilesWithPawns()) && (newPos == oldPos + 16 || newPos == oldPos - 16)) {
      epsquare = newPos - 8; // Set the en passant square to the square behind the pawn
    } else {
      epsquare = -1; // Reset en passant square if not a two-square pawn move
    }

    // Update playerToMove
    playerToMove ^= WHITE | BLACK;
    updateTilesWithSameColoredPieces();

  } else {
    printf("Illegal move: %s\n", getLongAlgebraicNotationFromPosition((short)position));
  }
}

void doLongAlgebraicNotationMove(char* notation) {
  int position = getPositionFromLongAlgebraicNotation(notation);
  int promotionType = 0;

  if(notation[4] == 'b') {
    promotionType = BISHOP;
  } else if (notation[4] == 'r') {
    promotionType = ROOK;
  } else if (notation[4] == 'k') {
    promotionType = KNIGHT;
  } else if (notation[4] == 'q') {
    promotionType = QUEEN;
  }

  doMove(position, promotionType);
}

void innit(char* FENPosition) {
  if(strcmp(FENPosition, "startPosition") == 0) {
    loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  } else {
    loadFEN(FENPosition);
  }

  innitTilesWithPieces(); // Initialize the tiles with pieces bitboards
  innitCheckBitBoards(); // Initialize the attacking bitboards for all pieces
  lastMove[0] = -1; // Initialize last move to -1 (no move made yet)
  lastMove[1] = -1; // Initialize last move to -1 (no move made yet)
}

int main() {
    uint64_t startingPos = ((uint64_t)1) << 15; // Example position for a piece, e.g., a knight on b3

    innit("startPosition");

    printBoard();

    calcAllLegalMoves();
    printPossibleMoves();
}

/* IDEAS:
  

*/