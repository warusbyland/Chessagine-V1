#include "Board.h"
#include <iostream>
#include "../MoveGen/moveGen.h"
#include "../../Chessagine.h"

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

void Board::loadFen(const std::string& fen) {
    for (int i = 0; i < PIECE_NB; ++i) pieceBB[i] = 0ULL;
    castlingRights = 0;
    moveHistory.clear();

    // parse the FEN string
    size_t pos = 0;
    int square = 63;

    // piece placement
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

    // 2  turn
    pos++;
    turn = fen[pos++] == 'w' ? WHITE : BLACK;

    // 3 castle rights
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

    // en pasasnt
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
std::string Board::getFen() const {
    std::string fen;

    // 1. Piece placement
    for (int rank = 7; rank >= 0; --rank) {
        int emptyCount = 0;

        for (int file = 7; file >= 0; --file) {
            int square = rank * 8 + file;
            char pieceChar = 0;

            for (int i = 0; i < PIECE_NB; ++i) {
                if (pieceBB[i] & (1ULL << square)) {
                    switch (i) {
                        case WHITE_PAWN:   pieceChar = 'P'; break;
                        case WHITE_KNIGHT: pieceChar = 'N'; break;
                        case WHITE_BISHOP: pieceChar = 'B'; break;
                        case WHITE_ROOK:   pieceChar = 'R'; break;
                        case WHITE_QUEEN:  pieceChar = 'Q'; break;
                        case WHITE_KING:   pieceChar = 'K'; break;
                        case BLACK_PAWN:   pieceChar = 'p'; break;
                        case BLACK_KNIGHT: pieceChar = 'n'; break;
                        case BLACK_BISHOP: pieceChar = 'b'; break;
                        case BLACK_ROOK:   pieceChar = 'r'; break;
                        case BLACK_QUEEN:  pieceChar = 'q'; break;
                        case BLACK_KING:   pieceChar = 'k'; break;
                    }
                    break;
                }
            }

            if (pieceChar) {
                if (emptyCount > 0) {
                    fen += std::to_string(emptyCount);
                    emptyCount = 0;
                }
                fen += pieceChar;
            } else {
                emptyCount++;
            }
        }

        if (emptyCount > 0)
            fen += std::to_string(emptyCount);

        if (rank > 0)
            fen += '/';
    }

    // 2. Turn
    fen += ' ';
    fen += (turn == WHITE) ? 'w' : 'b';

    // 3. Castling rights
    fen += ' ';
    std::string castlingStr;
    if (castlingRights & WHITE_KINGSIDE)  castlingStr += 'K';
    if (castlingRights & WHITE_QUEENSIDE) castlingStr += 'Q';
    if (castlingRights & BLACK_KINGSIDE)  castlingStr += 'k';
    if (castlingRights & BLACK_QUEENSIDE) castlingStr += 'q';
    fen += castlingStr.empty() ? "-" : castlingStr;

    // 4. En passant
    fen += ' ';
    if (epSq == -1) {
        fen += '-';
    } else {
        int sq = epSq - 1;
        char file = 'a' + (sq % 8);
        char rank = '1' + (sq / 8);
        fen += file;
        fen += rank;
    }

    // 5. Halfmove clock and fullmove number (default 0/1 if not tracked)
    fen += " 0 1";

    return fen;
}

Piece Board::getPiece(const int sq) const {
    uint64_t bitmask = 1ULL << sq;

    for (int i = 0; i < PIECE_NB; ++i) {
        if (pieceBB[i] & bitmask) { return static_cast<Piece>(i); }
    }

    return PIECE_NB;
}
void Board::updateCache() {
    occupancies[WHITE] = pieceBB[0] | pieceBB[1] | pieceBB[2] | pieceBB[3] | pieceBB[4] | pieceBB[5];
    occupancies[BLACK] = pieceBB[6] | pieceBB[7] | pieceBB[8] | pieceBB[9] | pieceBB[10] | pieceBB[11];
    occupancies[BOTH] = occupancies[WHITE] | occupancies[BLACK];

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
bool Board::isKingInCheck() const {
    return getKingBB(turn) & atkSq[!turn];
}

void Board::playerMove(int from, int to, Flag promo = QUIET) {
    Moves moves = MoveGen::genLegalMoves(*this);
    if (moves.empty()) return;

    for (Move m : moves) {
        if (fromSquare(m) == from && toSquare(m) == to) {
            move(m);
            Chessagine::think(*this, 5);
            return;
        }
    }
}
void Board::move(Move m) {
    const int from = fromSquare(m);
    const int to = toSquare(m);
    const Flag f = moveFlag(m);
    GameInfo info;

    // Save current state
    memcpy(info.pieceBB, pieceBB, sizeof(pieceBB));
    memcpy(info.occupancies, occupancies, sizeof(occupancies));
    memcpy(info.atkSq, atkSq, sizeof(atkSq));
    info.epSq = epSq;
    info.castlingRights = castlingRights;

    moveHistory.push_back(info);

    // Move logic
    const Piece piece = getPiece(from);
    const BB toMask = bm(to);

    if (f == DOUBLE_PUSH) {
        epSq = to + (turn == WHITE ? -8 : 8);
    } else { epSq = -1; }

    if (piece == WHITE_KING) {
        castlingRights &= ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
    } else if (piece == BLACK_KING) {
        castlingRights &= ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
    } else if (piece == WHITE_ROOK || piece == BLACK_ROOK)
        castlingRights &= ~ROOKCASTLELOSS[from];

    switch (f) {
        case EN_PASSANT: {
            if (turn == WHITE) {
                pieceBB[BLACK_PAWN] &= ~bm(to - 8);
            } else {
                pieceBB[WHITE_PAWN] &= ~bm(to + 8);
            }
            break;
        }
        case KING_CASTLE: {
            if (turn == WHITE) {
                pieceBB[WHITE_ROOK] &= ~1ULL;
                pieceBB[WHITE_ROOK] |= bm(2);
                castlingRights &= ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
            } else {
                pieceBB[BLACK_ROOK] &= ~bm(56);
                pieceBB[BLACK_ROOK] |= bm(58);
                castlingRights &= ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
            }
            break;
        }
        case QUEEN_CASTLE: {
            if (turn == WHITE) {
                pieceBB[WHITE_ROOK] &= ~bm(7);
                pieceBB[WHITE_ROOK] |= bm(4);
                castlingRights &= ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
            } else {
                pieceBB[BLACK_ROOK] &= ~bm(63);
                pieceBB[BLACK_ROOK] |= bm(60);
                castlingRights &= ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
            }
            break;
        }
        case CAPTURE: {
            Piece enemy = getPiece(to);
            pieceBB[enemy] &= ~toMask;
            if (enemy == WHITE_ROOK || enemy == BLACK_ROOK)
                castlingRights &= ~ROOKCASTLELOSS[to];
            break;
        }
        case PROMO_CAPTURE_N: {
            pieceBB[getPiece(to)] &= ~toMask;          
            pieceBB[getKnight(turn)] |= toMask;        
            break;
        }
        case PROMO_CAPTURE_B: {
            pieceBB[getPiece(to)] &= ~toMask;
            pieceBB[getBishop(turn)] |= toMask;
            break;
        }
        case PROMO_CAPTURE_R: {
            pieceBB[getPiece(to)] &= ~toMask;
            pieceBB[getRook(turn)] |= toMask;
            break;
        }
        case PROMO_CAPTURE_Q: {
            pieceBB[getPiece(to)] &= ~toMask;
            pieceBB[getQueen(turn)] |= toMask;
            break;
        }
        case PROMO_N: {
            pieceBB[getKnight(turn)] |= toMask;
            break;
        }
        case PROMO_B: {
            pieceBB[getBishop(turn)] |= toMask;
            break;
        }
        case PROMO_R: {
            pieceBB[getRook(turn)] |= toMask;
            break;
        }
        case PROMO_Q: {
            pieceBB[getQueen(turn)] |= toMask;
            break;
        }
    }

    pieceBB[piece] &= ~bm(from);
    if (f < 6) {
        pieceBB[piece] |= toMask;
    }

    updateCache();
    turn = !turn;
}
void Board::undo() {
    if (moveHistory.empty())
        return;

    GameInfo last = moveHistory.back();
    moveHistory.pop_back();

    // Restore state
    memcpy(pieceBB, last.pieceBB, sizeof(pieceBB));
    memcpy(occupancies, last.occupancies, sizeof(occupancies));
    memcpy(atkSq, last.atkSq, sizeof(atkSq));
    epSq = last.epSq;
    castlingRights = last.castlingRights;

    updateCache();
    turn = !turn;
}
