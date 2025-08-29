#pragma once

#include <cstdint> 
#include <string>
#include "../moves.h"

struct GameState {
    BB pieceBB[PIECE_NB];
    BB occupancies[COLOR_NB];
    Color turn;
    int8_t epSq;
    uint8_t castlingRights;
};
struct MoveInfo {
    GameState state;
    int from;
    int to;
    int capturedPiece; // PIECE_NONE if no capture
};

class Board {
public:
    Board(const std::string& fen);
    Piece getPiece(int sq);

    inline Color getTurn() { return turn; }

    void updateCache();
    inline BB getOccupancy(Color c) { return occupancies[c]; }
    inline BB getAllOccupancy()  { return occupancies[BOTH]; }
    inline BB getAttackedBy(Color c) { return atkSq[c]; }

    inline BB getPieceBoard(Piece piece) { return pieceBB[piece]; }
    inline BB getPawnBB(Color c)   { return pieceBB[c == WHITE ? WHITE_PAWN   : BLACK_PAWN]; }
    inline BB getKnightBB(Color c) { return pieceBB[c == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT]; }
    inline BB getBishopBB(Color c) { return pieceBB[c == WHITE ? WHITE_BISHOP : BLACK_BISHOP]; }
    inline BB getRookBB(Color c)   { return pieceBB[c == WHITE ? WHITE_ROOK   : BLACK_ROOK]; }
    inline BB getQueenBB(Color c)  { return pieceBB[c == WHITE ? WHITE_QUEEN  : BLACK_QUEEN]; }
    inline BB getKingBB(Color c)   { return pieceBB[c == WHITE ? WHITE_KING   : BLACK_KING]; }

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

    inline uint8_t getEnpassantSq() { return epSq; }
    inline uint8_t castleRights() { return castlingRights; } 

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

    std::vector<MoveInfo> moveHistory;
};