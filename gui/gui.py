import ctypes
import os
import sys
import platform
from PyQt5.QtWidgets import *

class ChessGUI(QWidget):
    def __init__(self, FENPosition="startPosition", parent=None):
        super().__init__(parent)
        self.setWindowTitle("Chess GUI")
        self.layout = QGridLayout()
        self.setLayout(self.layout)
        self.load_library()
        self.lib.innit(FENPosition.encode("utf-8"))  # Initialize the chess logic library
        self.initUI()
        self.tests()  # Example usage of the library function

    def initUI(self):
        # Initialize the GUI components here
        pass

    def load_library(self):
        if platform.system() == "Windows":
            libname = "chess.dll"
        elif platform.system() == "Darwin":  # macOS
            libname = "libchess.dylib"
        else:
            libname = "libchess.so"

        path = os.path.abspath("./out/" + libname)

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

        self.lib = lib

    def printPossibleMoves(self):
        self.moves = self.getPossibleMoves()
        for idx, move in enumerate(self.moves, 1):
            print(f"{idx}: {self.getLongAlgebraicNotationFromPosition(move)}")

    def getPossibleMoves(self):
        values = self.lib.getPossibleMoves()
        return [values[i] for i in range(0, 218) if values[i] != 0]

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

if __name__ == "__main__":
    app = QApplication(sys.argv)
    gui = ChessGUI()
    gui.show()
    sys.exit(app.exec_())