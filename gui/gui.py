import ctypes
import os
import sys
import platform
from PyQt5.QtWidgets import *
from PyQt5.QtGui import QPixmap

class ChessGUI(QWidget):
    def __init__(self, FENPosition="startPosition", parent=None):
        super().__init__(parent)
        self.setWindowTitle("Chess GUI")
        self.width = 800
        self.height = 800
        self.board = [0] * 64  # Initialize the board with empty squares
        self.circles = [[]]  # List to hold circle labels for possible moves
        self.pieces = [[]]  # List to hold piece labels
        self.oldPos = -1  # Variable to hold the last clicked position
        self.setFixedSize(self.width, self.height)  # Window is now not resizable

        self.load_library()
        self.lib.innit(FENPosition.encode("utf-8"))  # Initialize the chess logic library

        self.initUI()

    def initUI(self):
        # Background label
        bg_path = os.path.abspath("./assets/chessboard.png").replace("\\", "/")
        bg_label = QLabel(self)
        bg_label.setPixmap(QPixmap(bg_path))
        bg_label.setScaledContents(True)
        bg_label.setGeometry(0, 0, self.width, self.height)

        # Layout on top
        self.layout = QGridLayout(self)
        self.setLayout(self.layout)

        for i in range(8):
            self.layout.setRowMinimumHeight(i, 100)
            self.layout.setRowStretch(i, 1)
            self.layout.setColumnMinimumWidth(i, 100)
            self.layout.setColumnStretch(i, 1)
        
        self.drawPieces()

    def mousePressEvent(self, event):
        position = (7 - event.x() // 100) + (7 - event.y() // 100) * 8

        if(position in [circle[0] if len(circle) > 0 else -1 for circle in self.circles]):
            move = (position << 6) + self.oldPos
            self.movePiece(self.oldPos, position)
            self.doMove(move)
            self.deleteCircles()
            self.board = self.getBoard()  # Update the board state after the move
            return

        self.oldPos = position
        moves = self.getPossibleMovesOfPosition(position)

        self.deleteCircles()  # Clear previous circles

        for move in moves:
            x = 7 - ((move >> 6) & 63) % 8
            y = 7 - ((move >> 6) & 63) // 8
            circle = QLabel(self)
            
            if self.board[(move >> 6) & 63] == 0:
                circle_size = 30  # or 80 for even bigger circles
                offset = (100 - circle_size) // 2
                circle.setStyleSheet(f"background: rgba(0, 0, 0, 120); border-radius: {circle_size // 2}px;")
            else:
                circle_size = 70  # or 80 for even bigger circles
                offset = (100 - circle_size) // 2
                circle.setStyleSheet(f"""
                    background: transparent;
                    border: 2px solid rgba(220, 20, 60, 180);
                    border-radius: {circle_size // 2}px;
                """)

            circle.setGeometry(x * 100 + offset, y * 100 + offset, circle_size, circle_size)
                
            circle.show()
            self.circles.append([(7-x)+((7-y)*8), circle])

    def drawPieces(self):
        self.board = self.getBoard()
        for i in range(64):
            match self.board[i]:
                case 65: piece = "pawnWhite"
                case 66: piece = "knightWhite"
                case 68: piece = "bishopWhite"
                case 72: piece = "rookWhite"
                case 80: piece = "queenWhite"
                case 96: piece = "kingWhite"
                case 129: piece = "pawnBlack"
                case 130: piece = "knightBlack"
                case 132: piece = "bishopBlack"
                case 136: piece = "rookBlack"
                case 144: piece = "queenBlack"
                case 160: piece = "kingBlack"
                case _ : piece = "0"

            if piece != "0":
                self.addPiece(i, piece)

    def addPiece(self, position, piece):
        x = 7 - position % 8
        y = 7 - position // 8
        piece_path = os.path.abspath(f"./assets/{piece}.png").replace("\\", "/")
        piece_label = QLabel(self)
        piece_label.setPixmap(QPixmap(piece_path))
        piece_label.setScaledContents(True)
        piece_label.setGeometry(x * 100, y * 100, 100, 100)
        piece_label.show()
        self.pieces.append([position, piece_label])

    def deletePiece(self, position):
        for piece in self.pieces:
            if len(piece) > 1 and piece[0] == position:
                piece[1].deleteLater()
                self.pieces = [p for p in self.pieces if len(p) > 0 and p[0] != position]
                return

    def movePiece(self, old_position, new_position):
        for piece in self.pieces:
            if len(piece) > 1 and piece[0] == old_position:
                for piece2 in self.pieces:
                    if len(piece2) > 1 and piece2[0] == new_position:
                        self.deletePiece(new_position)  # Remove piece if it already exists at new position
                piece[1].setGeometry((7 - new_position % 8) * 100, (7 - new_position // 8) * 100, 100, 100)
                piece[0] = new_position

    def deleteCircles(self):
        for circle in self.circles:
            if len(circle) > 1:
                circle[1].deleteLater()
        self.circles = [[]]  # Clear the circles list

    def load_library(self):
        if platform.system() == "Windows":
            libname = "chess.dll"
        elif platform.system() == "Darwin":  # macOS
            libname = "libchess.dylib"
        else:
            libname = "libchess.so"

        path = os.path.abspath("./logic/" + libname)

        if not os.path.exists(path):
            raise FileNotFoundError(f"The library {libname} does not exist in the expected path: {path}")

        # load DLL/SO/DYLIB
        lib = ctypes.CDLL(path)

        lib.innit.argtypes = []
        lib.innit.restype = None

        lib.getLongAlgebraicNotationFromPosition.argtypes = [ctypes.c_short]
        lib.getLongAlgebraicNotationFromPosition.restype = ctypes.c_char_p

        lib.getPositionFromLongAlgebraicNotation.argtypes = [ctypes.c_char_p]
        lib.getPositionFromLongAlgebraicNotation.restype = ctypes.c_int

        lib.getPossibleMoves.argtypes = []
        lib.getPossibleMoves.restype = ctypes.POINTER(ctypes.c_ushort)

        lib.doMove.argtypes = [ctypes.c_int]
        lib.doMove.restype = None

        lib.doLongAlgebraicNotationMove.argtypes = [ctypes.c_char_p]
        lib.doLongAlgebraicNotationMove.restype = None

        lib.isCheckMate.argtypes = []
        lib.isCheckMate.restype = ctypes.c_int

        lib.getBoard.argtypes = []
        lib.getBoard.restype = ctypes.POINTER(ctypes.c_uint)

        self.lib = lib

    def printPossibleMoves(self):
        self.moves = self.getPossibleMoves()
        for idx, move in enumerate(self.moves, 1):
            print(f"{idx}: {self.getLongAlgebraicNotationFromPosition(move)}")

    def getPossibleMoves(self):
        values = self.lib.getPossibleMoves()
        return [values[i] for i in range(0, 218) if values[i] != 0]
    
    def getPossibleMovesOfPosition(self, pos: int):
        moves = self.lib.getPossibleMoves()
        moves = [moves[i] for i in range(0, 218) if moves[i] != 0]
        return [move for move in moves if (move & 63) == pos]
    
    def getLongAlgebraicNotationFromPosition(self, pos: int):
        return (self.lib.getLongAlgebraicNotationFromPosition(pos)).decode("utf-8")
    
    def getPositionFromLongAlgebraicNotation(self, notation: str):
        return self.lib.getPositionFromLongAlgebraicNotation(notation.encode("utf-8"))
    
    def doMove(self, pos: int):
        self.lib.doMove(pos)
    
    def doLongAlgebraicNotationMove(self, notation: str):
        self.lib.doLongAlgebraicNotationMove(notation.encode("utf-8"))

    def isCheckMate(self):
        return self.lib.isCheckMate() == 0
    
    def getBoard(self):
        board_ptr = self.lib.getBoard()
        board = [board_ptr[i] for i in range(64)]
        return board

if __name__ == "__main__":
    app = QApplication(sys.argv)
    gui = ChessGUI()
    gui.show()
    sys.exit(app.exec_())

"""
    TODO: handle promotion, handle castling

"""