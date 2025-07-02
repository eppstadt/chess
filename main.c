#include <stdint.h>
#include <stdio.h>
#include <intrin.h>

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
} Piece;

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
  if(playerToMove == 8) {
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

/**
 * @brief Prints the current state of the chess board.
 * 
 * This function iterates through the board array and prints the piece type at each position.
 * It formats the output to display the board in an 8x8 grid.
 */
void printBoard() {
    for(int i = 1; i < 65; i++) {
        printf("%d ", board[i-1].pieceType);
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
uint64_t getPossibleMoveBitBoardPawn(uint64_t startingPos) {
  uint64_t possibleMoves = 0;

  if(playerToMove == WHITE && isNotOn(startingPos, eigthRank)) {
    if(isNotOn(startingPos, hFile) && isOn(startingPos << 7, tilesWithOppositeColoredPieces | (((uint64_t)1) << epsquare))) {
      possibleMoves |= startingPos << 7;
    }
    if(isNotOn(startingPos, aFile) && isOn(startingPos << 9, tilesWithOppositeColoredPieces | (((uint64_t)1) << epsquare))) {
      possibleMoves |= startingPos << 9;
    }
    if(isNotOn(startingPos << 8, tilesWithPieces())) {
      possibleMoves |= startingPos << 8;
      if(isOn(startingPos, secondRank) && isNotOn(startingPos << 16, tilesWithPieces())) {
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
    if(isNotOn(startingPos >> 8, tilesWithPieces())) {
      possibleMoves |= startingPos >> 8;
      if(isOn(startingPos, seventhRank) && isNotOn(startingPos >> 16, tilesWithPieces())) {
        possibleMoves |= startingPos >> 16;
      }
    }
  }

  return possibleMoves;
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
uint64_t getPossibleMoveBitBoardKing(uint64_t startingPos) {
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

  if((castlingAbility[0] || castlingAbility[2]) && isNotOn(startingPos >> 1, tilesWithPieces()) && isNotOn(startingPos >> 2, tilesWithPieces())) {
    possibleMoves |= startingPos >> 2; // kingside castling
  }

  if((castlingAbility[1] || castlingAbility[3]) && isNotOn(startingPos << 1, tilesWithPieces()) && isNotOn(startingPos << 2, tilesWithPieces())) {
    possibleMoves |= startingPos << 2; // queenside castling
  }

  return possibleMoves;
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
uint64_t getPossibleMoveBitBoard(int position) {

  uint64_t possibleMoves = 0;
  uint64_t startingPos = ((uint64_t)1) << position;

  int pieceType = board[position].pieceType & 7; // Mask to get the piece type without color

  switch(pieceType) {
    case PAWN:
      possibleMoves = getPossibleMoveBitBoardPawn(startingPos);
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
      possibleMoves = getPossibleMoveBitBoardKing(startingPos);
      break;
  }

  return possibleMoves;
}

/**
 * @brief Generates a bitboard representing the squares where the balck king is in check.
 * 
 * This function iterates through all pieces on the board, checking if they are white pieces
 * and if they can attack the square of the black king. It updates the attackingBitBoard for
 * each white- QUEEN/ROOK/BISHOP and the current piece move and accumulates the squares where the black king is in check.
 * 
 * @return uint64_t A bitboard representing the squares where the black king is in check.
 */
uint64_t generateWhiteCheckBitBoard() {
  uint64_t inWhiteCheckTMP = 0;

  for(int i = 0; i < 64; i++) {
    if(board[i].pieceType == 0 || board[i].pieceType > 15) continue;
    if(board[i].pieceType == (QUEEN | WHITE) || board[i].pieceType == (ROOK | WHITE) || board[i].pieceType == (BISHOP | WHITE) || i == lastMove[1]) {
      board[i].attackingBitBoard = getPossibleMoveBitBoard(i);
    }
    inWhiteCheckTMP |= board[i].attackingBitBoard;
  }

  return inWhiteCheckTMP;
}

/**
 * @brief Generates a bitboard representing the squares where the white king is in check.
 * 
 * This function iterates through all pieces on the board and updates the attackingBitBoard for
 * each black- QUEEN/ROOK/BISHOP and the current piece move and accumulates the squares where the white king is in check.
 * 
 * @return uint64_t A bitboard representing the squares where the white king is in check.
 */
uint64_t generateBlackCheckBitBoard() {
  uint64_t inBlackCheckTMP = 0;

  for(int i = 0; i < 64; i++) {
    if(board[i].pieceType < 16) continue;
    if(board[i].pieceType == (QUEEN | BLACK) || board[i].pieceType == (ROOK | BLACK) || board[i].pieceType == (BISHOP | BLACK) || i == lastMove[1]) {
      board[i].attackingBitBoard = getPossibleMoveBitBoard(i);
    }
    inBlackCheckTMP |= board[i].attackingBitBoard;
  }

  return inBlackCheckTMP;
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
bool simultateMove(int oldPos, int newPos) {

  bool invalidMove = false;

  // Update the board
  Piece temp = board[newPos];
  board[newPos] = board[oldPos];
  board[oldPos] = (Piece){0, 0};

  // Update the last move
  int oldLastMove[2];
  oldLastMove[0] = lastMove[0];
  oldLastMove[1] = lastMove[1];
  
  lastMove[0] = oldPos;
  lastMove[1] = newPos;

  // Update the tiles with pieces
  if(board[newPos].pieceType & WHITE) {
    tilesWithWhitePieces &= ~(((uint64_t)1) << oldPos);
    tilesWithWhitePieces |= (((uint64_t)1) << newPos);
    if(generateBlackCheckBitBoard() & tilesWithWhiteKings) { invalidMove = true; };
  } else {
    tilesWithBlackPieces &= ~(((uint64_t)1) << oldPos);
    tilesWithBlackPieces |= (((uint64_t)1) << newPos);
    if(generateWhiteCheckBitBoard() & tilesWithBlackKings) { invalidMove = true; };
  }

  // Undo the move
  board[oldPos] = board[newPos];
  board[newPos] = temp;

  lastMove[0] = oldLastMove[0];
  lastMove[1] = oldLastMove[1];

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
bool moveIsLegal(int oldPos, int newPos) {
  // Check if the move is valid
  if(oldPos < 0 || oldPos >= 64 || newPos < 0 || newPos >= 64) return false;

  if(oldPos == newPos) return false; // No move made

  if(board[oldPos].pieceType == 0) return false; // No piece at old position

  if(board[oldPos].pieceType & 24 == playerToMove) return false; // Piece is not of the current player
  
  // Check if the piece is moving to a square occupied by a piece of the same color
  if((board[oldPos].pieceType & WHITE) == (board[newPos].pieceType & WHITE)) return false;

  // Simulate the move and check if it results in the king being in check
  return !simultateMove(oldPos, newPos);
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

  for(int i = 0; i < 64; i++) {
    if(board[i].pieceType == 0 || board[i].pieceType & playerToMove == 0) continue; // No piece of the right color at this position

    uint64_t possibleMoveBitBoard = getPossibleMoveBitBoard(i);

    while(possibleMoveBitBoard) {
      int movePos = ctzll(possibleMoveBitBoard); // Get the index of the least significant bit
      possibleMoveBitBoard &= ~(1ULL << movePos); // Clear the bit at movePos

      // Check if the move is legal
      if(moveIsLegal(i, movePos)) {
        possibleMoves[moveIndex++] = (i << 8) | movePos; // Store the move in the possibleMoves array
      }
    }
  }

  possibleMoves[moveIndex] = 0; // Mark the end of the moves
}

/**
 * @brief Counts the number of trailing zeros in a 64-bit integer.
 * 
 * This function uses the _BitScanForward64 intrinsic to find the index of the least significant bit that is set.
 * If the input is zero, it returns -1 to indicate an undefined result.
 * 
 * @param mask The 64-bit integer to check.
 * @return int The index of the least significant bit that is set, or -1 if the input is zero.
 * @note This function is specific to Windows x64 and uses the _BitScanForward64 intrinsic. 
 */
int ctzll(uint64_t mask) {
    unsigned long index;
    if (_BitScanForward64(&index, mask))
        return (int)index;
    return -1; // Undefined if mask == 0
}

char* getLongAlgebraicNotationFromPosition(short position) {
  static char notationBuffer[6];

  int newPos = position & 63; // Mask to get the position in the range 0-63
  int oldPos = (position >> 6) & 63; // Extract the old position from the higher bits

  char promotingPiece = '\0';
  if (position & 0x100) {
    promotingPiece = 'b'; // Promotion to Bishop
  } else if (position & 0x200) {
    promotingPiece = 'r'; // Promotion to Rook
  } else if (position & 0x300) {
    promotingPiece = 'k'; // Promotion to Knight
  } else if (position & 0x400) {
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
    position |= 0x100; // Bishop promotion
  } else if(notation[4] == 'r') {
    position |= 0x200; // Rook promotion
  } else if(notation[4] == 'k') {
    position |= 0x300; // Knight promotion
  } else if(notation[4] == 'q') {
    position |= 0x400; // Queen promotion
  }

  return position;
}

void printPossibleMoves() {
  for(int i = 0; i < 218; i++) {
    if(possibleMoves[i] == 0) break; // End of moves
    printf("%d: %s\n", i, getLongAlgebraicNotationFromPosition((short)possibleMoves[i]));
  }
}

void doMove(int position) {
  int oldPos = (position >> 8) & 63; // Extract the old position from the higher bits
  int newPos = position & 63; // Mask to get the new position in the range 0-63

  if(moveIsLegal(oldPos, newPos)) {
    board[newPos] = board[oldPos];
    board[oldPos] = (Piece){0, 0};

    lastMove[0] = oldPos;
    lastMove[1] = newPos;

    if(board[newPos].pieceType & WHITE) {
      tilesWithWhitePieces &= ~(((uint64_t)1) << oldPos);
      tilesWithWhitePieces |= (((uint64_t)1) << newPos);
      inBlackCheck = generateWhiteCheckBitBoard();
    } else {
      tilesWithBlackPieces &= ~(((uint64_t)1) << oldPos);
      tilesWithBlackPieces |= (((uint64_t)1) << newPos);
      inWhiteCheck = generateBlackCheckBitBoard();
    }

    //TODO: some more variables need to be updated, like castling rights, en passant square, halfmove clock, fullmove number

    // Update playerToMove
    playerToMove ^= WHITE | BLACK;
  } else {
    printf("Illegal move: %s\n", getLongAlgebraicNotationFromPosition((short)position));
  }
}

void doLongAlgebraicNotationMove(char* notation) {
  int position = getPositionFromLongAlgebraicNotation(notation);
  doMove(position);
}

void innit(char* FENPosition) {
  if(strcmp(FENPosition, "startPosition") == 0) {
    loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  } else {
    loadFEN(FENPosition);
  }
}

int main() {
    uint64_t startingPos = ((uint64_t)1) << 15; // Example position for a piece, e.g., a knight on b3

    loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    printBoard();
    printBitmask(getPossibleMoveBitBoardRook(startingPos));
    printBitmask(getPossibleMoveBitBoard(1));

    possibleMoves[0] = (2 << 6) | 19; // Example move from f1 to e3
    possibleMoves[1] = 0; // End of moves
    printPossibleMoves();
}