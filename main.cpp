#include <iostream>

using namespace std;

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
bool castlingAbility[4];
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
            continue;
        }
        
        if(*position > 48 && *position < 57) {
            row += *position - 48;
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
    }

    cout << "HI";

    position++;
    playerToMove = *position == 'w'? WHITE : BLACK;
    position += 2;

    for (; *position != ' '; position++) {
        switch (*position) {
            case '-':
                break;
            case 'k':
                castlingAbility[0] = true;
                break;
            case 'q':
                castlingAbility[1] = true;
                break;
            case 'K':
                castlingAbility[2] = true;
                break;
            case 'Q':
                castlingAbility[3] = true;
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

    int i = 0;

    for(char piece : board) {
        cout << piece << " ";
        i++;
        if(i % 8 == 0) cout << "\n";
    }
}

int main() {
    cout << "Test";
    loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    printBoard();
}