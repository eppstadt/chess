import subprocess
import os
import atexit

class Rewards:
    def __init__(self):
        path = os.path.abspath("./stockfish/stockfish-windows-x86-64-avx2.exe")
        if not os.path.exists(path):
            raise FileNotFoundError(f"Stockfish executable not found at {path}")
        startupinfo = None
        if os.name == "nt":
            startupinfo = subprocess.STARTUPINFO()
            startupinfo.dwFlags &= ~subprocess.STARTF_USESHOWWINDOW
        self.process = subprocess.Popen(
            [path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        atexit.register(self.close)

    def getReward(self, Position, isWhite=True):
        self.process.stdin.write(f"position {Position}\n")
        self.process.stdin.write(f"eval\n")
        self.process.stdin.flush()
        
        while not self.process.stdout.readline().startswith("NNUE evaluation"):
            pass

        return float(self.process.stdout.readline().split()[2]) * (1 if isWhite else -1)

    def close(self):
        if self.process:
            self.process.stdin.write("quit\n")
            self.process.stdin.flush()
            self.process.terminate()
            self.process.wait()
            self.process = None
    
if __name__ == "__main__":
    rewards = Rewards()
    position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"
    reward = rewards.getReward(position)
    print(f"Reward for position {position}: {reward}")