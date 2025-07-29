import ctypes
import os
import sys
import platform
from PyQt5.QtWidgets import *
from PyQt5.QtGui import QPixmap, QIcon

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
        self.updateGameState()
        types = {"queenWhite": 0x8000, "rookWhite": 0x2000, "bishopWhite": 0x1000, "knightWhite": 0x4000, "queenBlack": 0x8000, "rookBlack": 0x2000, "bishopBlack": 0x1000, "knightBlack": 0x4000}
        position = (7 - event.x() // 100) + (7 - event.y() // 100) * 8

        if(position in [circle[0] if len(circle) > 0 else -1 for circle in self.circles]):
            move = (position << 6) + self.oldPos
            piece = None

            if(self.board[self.oldPos] & 32 and abs(position - self.oldPos) == 2):
                    # Handle castling
                    rook_old_position = (0 if self.oldPos in [3,59] else 7) + (self.oldPos // 8) * 8
                    rook_new_position = (2 if self.oldPos in [3,59] else 5) + (self.oldPos // 8) * 8
                    self.movePiece(rook_old_position, rook_new_position)
            if(self.board[self.oldPos] & 1 and (position // 8 == 0 or position // 8 == 7)):
                color = "White" if position // 8 == 7 else "Black"
                piece = self.promotionWindow(color)

            self.movePiece(self.oldPos, position, piece)
            self.doMove(move | (types.get(piece) if piece else 0))  # Perform the move in the chess logic library
            self.deleteCircles()
            self.board = self.getBoard()  # Update the board state after the move
            return

        self.oldPos = position
        moves = self.getPossibleMovesOfPosition(position)

        self.deleteCircles()

        for move in moves:
            x = 7 - ((move >> 6) & 63) % 8
            y = 7 - ((move >> 6) & 63) // 8
            circle = QLabel(self)
            
            if self.board[(move >> 6) & 63] == 0:
                circle_size = 30 
                offset = (100 - circle_size) // 2
                circle.setStyleSheet(f"background: rgba(0, 0, 0, 120); border-radius: {circle_size // 2}px;")
            else:
                circle_size = 70 
                offset = (100 - circle_size) // 2
                circle.setStyleSheet(f"""
                    background: transparent;
                    border: 2px solid rgba(220, 20, 60, 180);
                    border-radius: {circle_size // 2}px;
                """)

            circle.setGeometry(x * 100 + offset, y * 100 + offset, circle_size, circle_size)
                
            circle.show()
            self.circles.append([(7-x)+((7-y)*8), circle])

    def promotionWindow(self, color="White"):
        dialog = QDialog(self)
        dialog.setWindowTitle("Choose Promotion Piece")
        layout = QVBoxLayout(dialog)

        pieces = {"Queen": f"queen{color}", "Rook": f"rook{color}", "Bishop": f"bishop{color}", "Knight": f"knight{color}"}
        buttons = []

        for name, img in pieces.items():
            btn = QPushButton(name)
            icon_path = os.path.abspath(f"./assets/{img}.png").replace("\\", "/")
            btn.setIcon(QIcon(icon_path))
            #btn.setIconSize(QIcon(icon_path).pixmap(QSize(80, 80)).size())
            layout.addWidget(btn)
            buttons.append(btn)

        selected = {}

        def choose(piece):
            selected["piece"] = pieces[piece]
            dialog.accept()

        for btn, (name, _) in zip(buttons, pieces.items()):
            btn.clicked.connect(lambda _, n=name: choose(n))

        dialog.exec_()
        return selected.get("piece")

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

    def movePiece(self, old_position, new_position, promotion=None):
        for piece in self.pieces:
            if len(piece) > 1 and piece[0] == old_position:
                for piece2 in self.pieces:
                    if len(piece2) > 1 and piece2[0] == new_position:
                        self.deletePiece(new_position)  # Remove piece if it already exists at new position
                if(promotion != None):
                    self.deletePiece(old_position)  # Remove the old piece if promotion is selected
                    self.addPiece(new_position, promotion)  # Add the new promoted piece
                else:
                    piece[1].setGeometry((7 - new_position % 8) * 100, (7 - new_position // 8) * 100, 100, 100)
                    piece[0] = new_position

    def deleteCircles(self):
        for circle in self.circles:
            if len(circle) > 1:
                circle[1].deleteLater()
        self.circles = [[]]  # Clear the circles list

    def updateGameState(self):
        if self.getStaleMate():
            QMessageBox.information(self, "Game Over", "Stalemate! The game is a draw.")
        elif self.getCheckMate():
            QMessageBox.information(self, "Game Over", "Checkmate! The game is over.")

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

        lib.getStaleMate.argtypes = []
        lib.getStaleMate.restype = ctypes.c_int

        lib.getCheckMate.argtypes = []
        lib.getCheckMate.restype = ctypes.c_int

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

    def getStaleMate(self):
        return self.lib.getStaleMate() != 0
    
    def getCheckMate(self):
        return self.lib.getCheckMate() != 0
    
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
 TODO: Implement time control
"""