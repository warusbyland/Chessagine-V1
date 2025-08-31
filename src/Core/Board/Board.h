#pragma once

#include <cstdint> 
#include <string>
#include "../moves.h"

namespace FenUtility {
    const std::string startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
}

struct GameInfo {
    Color turn;
    BB pieceBB[PIECE_NB];
    BB occupancies[COLOR_NB];
    BB atkSq[BOTH];
    int8_t epSq;
    uint8_t castlingRights;
};

class Board {
public:
    void loadFen(const std::string& fen);
    Board(const std::string& fen = FenUtility::startpos) { loadFen(fen); }

    inline Color getTurn() const { return turn; }

    void updateCache();
    inline BB getOccupancy(Color c) const { return occupancies[c]; }
    inline BB getAllOccupancy() const { return occupancies[BOTH]; }
    inline BB getAttackedBy(Color c) const { return atkSq[c]; }

    Piece getPiece(int sq) const;
    inline BB getPieceBoard(Piece piece) const { return pieceBB[piece]; }
    inline BB getPawnBB(Color c) const { return pieceBB[c == WHITE ? WHITE_PAWN   : BLACK_PAWN]; }
    inline BB getKnightBB(Color c) const { return pieceBB[c == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT]; }
    inline BB getBishopBB(Color c) const { return pieceBB[c == WHITE ? WHITE_BISHOP : BLACK_BISHOP]; }
    inline BB getRookBB(Color c) const { return pieceBB[c == WHITE ? WHITE_ROOK   : BLACK_ROOK]; }
    inline BB getQueenBB(Color c) const { return pieceBB[c == WHITE ? WHITE_QUEEN  : BLACK_QUEEN]; }
    inline BB getKingBB(Color c) const { return pieceBB[c == WHITE ? WHITE_KING   : BLACK_KING]; }

    // Legality check movegen helpers
    void setPiece(const Piece p, const int sq) { 
        if (p == PIECE_NB) return;
        pieceBB[p] |= bm(sq); 
    } 
    void deletePiece(const Piece p, const int sq) { 
        if (p == PIECE_NB) return;
        pieceBB[p] &= ~bm(sq); 
    }

    bool isMoveSafe(Color us, int from, int to); 
    bool isKingInCheck() const;

    inline uint8_t getEnpassantSq() const { return epSq; }
    inline uint8_t castleRights() const { return castlingRights; } 

    void playerMove(int from, int to, Flag promo);
    void move(Move m);
    void undo();
private:
    BB pieceBB[PIECE_NB] = {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL};
    Color turn = WHITE;
    // First bit is a1, last is h8
    
    BB occupancies[COLOR_NB] = {0ULL, 0ULL, 0ULL}; // WHITE, BLACK, BOTH
    
    BB atkSq[BOTH] = {0ULL, 0ULL};
    // Piece board[64]; // Square -> Piece
    int8_t epSq = -1;


    uint8_t castlingRights = WHITE_KINGSIDE | WHITE_QUEENSIDE | BLACK_KINGSIDE | BLACK_QUEENSIDE;
    // --- To REMOVE a castling right --- 
    // castlingRights &= ~WHITE_KINGSIDE; 

    std::vector<GameInfo> moveHistory;
};