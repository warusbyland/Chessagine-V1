FILE_A = 0x0101010101010101
FILE_B = FILE_A << 1
FILE_G = FILE_A << 6
FILE_H = FILE_A << 7
NOT_A  = ~FILE_A
NOT_H  = ~FILE_H
NOT_AB  = NOT_A | ~FILE_B
NOT_GH  = NOT_H | ~FILE_G

kingmoves = []

def kingAttacks(kings):
    return (kings << 8) | (kings >> 8) | (((kings << 1) | (kings << 9) | (kings >> 7)) & NOT_A) | (((kings >> 1) | (kings << 7) | (kings >> 9)) & NOT_H)

def knightAttacks(knights):
    return (((knights << 15) | (knights >> 17)) & NOT_H) | (((knights << 6) | (knights >> 10)) & NOT_GH) | (((knights << 17) | (knights >> 15)) & NOT_A) | (((knights << 10) | (knights >> 6)) & NOT_AB)
def pawnAttacksW(pawns):
    return ((pawns << 7) & NOT_H) | ((pawns << 9) & NOT_A)

def pawnAttacksB(pawns):
    return ((pawns >> 9) & NOT_H) | ((pawns >> 7) & NOT_A)

for i in range(64):
    kingmoves.append(kingAttacks(1 << i) & 0xFFFFFFFFFFFFFFFF)

s = [" ", "x"]

def bm(i):
    return 0x1 << i
def showBin(BB):
    print("+---+---+---+---+---+---+---+---+")
    for rank in range(0, 64, 8):
        row = "| "
        for file in range(8):
            row += s[1 if BB & bm(rank + file) else 0]
            row += " | "
        print(row)
        print("+---+---+---+---+---+---+---+---+")

for i in range(64):
    print(kingmoves[i])