#include "Board.h"
#include <iostream>
#include "../MoveGen/moveGen.h"

void printBB(BB bb) {
    const char* s[2] = { " ", "x" };

    std::cout << "+---+---+---+---+---+---+---+---+\n";
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << "| ";
        for (int file = 7; file >= 0; --file) {
            int sq = rank * 8 + file;
            std::cout << s[(bb >> sq) & 1] << " | ";
        }
        std::cout << "\n+---+---+---+---+---+---+---+---+\n";
    }
}

Board::Board(const std::string& fen) {
    //parse the FEN string
    size_t pos = 0;
    int square = 63;

    //piece placement
    while (fen[pos] != ' ') {
        char c = fen[pos++];

        if (c == '/') {
            continue; // nove to next rank
        }

        if (isdigit(c)) {
            square -= (c - '0'); // skip empty squares
            continue;
        }

        uint64_t bit = 1ULL << square--;
        switch (c) {
            case 'P': pieceBB[WHITE_PAWN] |= bit; break;
            case 'N': pieceBB[WHITE_KNIGHT] |= bit; break;
            case 'B': pieceBB[WHITE_BISHOP] |= bit; break;
            case 'R': pieceBB[WHITE_ROOK] |= bit; break;
            case 'Q': pieceBB[WHITE_QUEEN] |= bit; break;
            case 'K': pieceBB[WHITE_KING] |= bit; break;
            case 'p': pieceBB[BLACK_PAWN] |= bit; break;
            case 'n': pieceBB[BLACK_KNIGHT] |= bit; break;
            case 'b': pieceBB[BLACK_BISHOP] |= bit; break;
            case 'r': pieceBB[BLACK_ROOK] |= bit; break;
            case 'q': pieceBB[BLACK_QUEEN] |= bit; break;
            case 'k': pieceBB[BLACK_KING] |= bit; break;
            default: break;
        }
    }

    // 2  skin color
    pos++;
    turn = fen[pos++] == 'w' ? WHITE : BLACK;

    // 3 human rights
    pos++; 
    castlingRights = 0ULL;
    while (fen[pos] != ' ') {
        switch (fen[pos++]) {
            case 'K': castlingRights |= WHITE_KINGSIDE; break;
            case 'Q': castlingRights |= WHITE_QUEENSIDE; break;
            case 'k': castlingRights |= BLACK_KINGSIDE; break;
            case 'q': castlingRights |= BLACK_QUEENSIDE; break;
            case '-': break;
        }
    }

    // Field 4: En passant target square
    if (fen[pos++] == '-') {
        epSq = -1;
        ++pos;
    } else {
        if (fen[pos] >= 'a' && fen[pos] <= 'h' &&
            fen[pos + 1] >= '1' && fen[pos + 1] <= '8') {

            int file = fen[pos] - 'a';          // A=0, B=1, ..., H=7
            int rank = fen[pos + 1] - '1';      // 1=0, 2=1, ..., 8=7
            epSq = rank * 8 + file + 1;

            pos += 2;
        } else {
            // Invalid FEN, you could throw or set epSq to -1
            epSq = -1;
        }
    }

    // // 5 Halfmove clock
    // pos++;
    // state.halfmoveClock = 0;
    // while (isdigit(fen[pos])) {
    //     state.halfmoveClock = state.halfmoveClock * 10 + (fen[pos++] - '0');
    // }

    // // 6 Fullmove number
    // pos++;
    // state.fullmoveNumber = 0;
    // while (isdigit(fen[pos])) {
    //     state.fullmoveNumber = state.fullmoveNumber * 10 + (fen[pos++] - '0');
    // }

    updateCache();
}

Piece Board::getPiece(const int sq) {
    uint64_t bitmask = 1ULL << sq;

    for (int i = 0; i < PIECE_NB; ++i) {
        if (pieceBB[i] & bitmask) { return static_cast<Piece>(i); }
    }

    return PIECE_NB;
}

void Board::updateCache() {
    occupancies[WHITE] = pieceBB[0] | pieceBB[1] | pieceBB[2] | pieceBB[3] | pieceBB[4] | pieceBB[5];
    occupancies[BLACK] = pieceBB[6] | pieceBB[7] | pieceBB[8] | pieceBB[9] | pieceBB[10] | pieceBB[11];
    occupancies[BOTH] = occupancies[0] | occupancies[1];

    atkSq[WHITE] =  MoveGen::attackedBy(*this, WHITE);
    atkSq[BLACK] =  MoveGen::attackedBy(*this, BLACK);
}

bool Board::isMoveSafe(Color us, int from, int to) {
    Piece enemy = getPiece(to);
    deletePiece(enemy, to);
    BB safe = ~MoveGen::attackedBy(*this, !us, occupancies[BOTH] & ~bm(from) | bm(to));
    setPiece(enemy, to);
    return getKingBB(us) & safe; 
}  

void Board::playerMove(int from, int to, Flag promo = QUIET) {
    std::string flagStrings[14] = {
        "QUIET","DOUBLE_PUSH","KING_CASTLE","QUEEN_CASTLE",
        "CAPTURE","EN_PASSANT","PROMO_KNIGHT","PROMO_BISHOP",
        "PROMO_ROOK","PROMO_QUEE","PROMO_CAPTURE_KNIGHT",
        "PROMO_CAPTURE_BISHOP","PROMO_CAPTURE_ROOK","PROMO_CAPTURE_QUEEN" 
    };
    std::string pieceStrings[PIECE_NB] = {
        "WHITE_PAWN", "WHITE_KNIGHT", "WHITE_BISHOP", "WHITE_ROOK", "WHITE_QUEEN", "WHITE_KING",
        "BLACK_PAWN", "BLACK_KNIGHT", "BLACK_BISHOP", "BLACK_ROOK", "BLACK_QUEEN", "BLACK_KING"
    };

    Moves moves = MoveGen::genLegalMoves(*this);

    for (int i = 0; i < moves.size(); i++) {
        const Move m = moves[i];
        const int fromM = fromSquare(m);
        const int toM = toSquare(m);
        std::string piece = pieceStrings[getPiece(fromSquare(m))];
        std::string flag = flagStrings[moveFlag(m)];

        std::cout << "Move "<< i + 1 << ": " << piece << " " << fromM << "-" << toM << " " << flag << "\n";
        if (fromM == from && toM == to) {
            move(m);
            return;
        }
    }
}

void Board::move(Move m) {
    const int from = fromSquare(m);
    const int to = toSquare(m);
    const Flag f = moveFlag(m);
    MoveInfo info;

    // Save current state
    memcpy(info.state.pieceBB, pieceBB, sizeof(pieceBB));
    memcpy(info.state.occupancies, occupancies, sizeof(occupancies));
    info.state.turn = turn;
    info.state.epSq = epSq;
    info.state.castlingRights = castlingRights;

    info.from = from;
    info.to = to;
    info.capturedPiece = getPiece(to); // Your own function

    moveHistory.push_back(info);

    // Move logic
    const Piece piece = getPiece(from);
    const BB toMask = bm(to);

    if (f == DOUBLE_PUSH) {
        epSq = to + (turn == WHITE ? -8 : 8);
    } else { epSq = -1; }

    switch (f) {
        case QUIET:
            if (piece == WHITE_KING) {
                castlingRights &= ~WHITE_KINGSIDE;
                castlingRights &= ~WHITE_QUEENSIDE;
            } else if (piece == BLACK_KING) {
                castlingRights &= ~BLACK_KINGSIDE;
                castlingRights &= ~BLACK_QUEENSIDE;
            } else if (piece == WHITE_ROOK || piece == BLACK_ROOK)
                castlingRights &= ROOKCASTLELOSS[from];
            break;
        case EN_PASSANT:
            if (turn == WHITE) {
                pieceBB[BLACK_PAWN] &= ~bm(to - 8);
            } else {
                pieceBB[WHITE_PAWN] &= ~bm(to + 8);
            }
            break;
        case KING_CASTLE:
            if (turn == WHITE) {
                castlingRights &= ~WHITE_KINGSIDE;
                pieceBB[WHITE_ROOK] &= ~1ULL;
                pieceBB[WHITE_ROOK] |= bm(2);
            } else {
                castlingRights &= ~BLACK_KINGSIDE;
                pieceBB[BLACK_ROOK] &= ~bm(56);
                pieceBB[BLACK_ROOK] |= bm(58);
            }
            break;
        case QUEEN_CASTLE:
            if (turn == WHITE) {
                castlingRights &= ~WHITE_QUEENSIDE;
                pieceBB[WHITE_ROOK] &= ~bm(7);
                pieceBB[WHITE_ROOK] |= bm(4);
            } else {
                castlingRights &= ~BLACK_QUEENSIDE;
                pieceBB[BLACK_ROOK] &= ~bm(63);
                pieceBB[BLACK_ROOK] |= bm(60);
            }
            break;
        case CAPTURE:
            pieceBB[getPiece(to)] &= ~toMask;
            break;
        case PROMO_CAPTURE_N:
            pieceBB[getPiece(to)] &= ~toMask;          
            pieceBB[getKnight(turn)] |= toMask;        
            break;
        case PROMO_CAPTURE_B:
            pieceBB[getPiece(to)] &= ~toMask;
            pieceBB[getBishop(turn)] |= toMask;
            break;
        case PROMO_CAPTURE_R:
            pieceBB[getPiece(to)] &= ~toMask;
            pieceBB[getRook(turn)] |= toMask;
            break;
        case PROMO_CAPTURE_Q:
            pieceBB[getPiece(to)] &= ~toMask;
            pieceBB[getQueen(turn)] |= toMask;
            break;
        case PROMO_N:
            pieceBB[getKnight(turn)] |= toMask;
            break;
        case PROMO_B:
            pieceBB[getBishop(turn)] |= toMask;
            break;
        case PROMO_R:
            pieceBB[getRook(turn)] |= toMask;
            break;
        case PROMO_Q:
            pieceBB[getQueen(turn)] |= toMask;
            break;
    }

    pieceBB[piece] &= ~bm(from);
    if (f < 6) {
        pieceBB[piece] |= toMask;
    }

    turn = !turn;
    updateCache();
}

void Board::undo() {
    if (moveHistory.empty())
        return;

    MoveInfo last = moveHistory.back();
    moveHistory.pop_back();

    // Restore state
    memcpy(pieceBB, last.state.pieceBB, sizeof(pieceBB));
    memcpy(occupancies, last.state.occupancies, sizeof(occupancies));
    turn = last.state.turn;
    epSq = last.state.epSq;
    castlingRights = last.state.castlingRights;
}
