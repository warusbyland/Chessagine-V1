#pragma once 

#include "../moves.h"
#include "../Board/Board.h"

using Score = int;

namespace Eval {

    constexpr Score INF = 1000000000;

    constexpr Score PAWN_VALUE   = 100;
    constexpr Score KNIGHT_VALUE = 320;
    constexpr Score BISHOP_VALUE = 330;
    constexpr Score ROOK_VALUE   = 500;
    constexpr Score QUEEN_VALUE  = 900;

    constexpr Score Piece_Materials[5] = { PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE };
    // constexpr int KING_VALUE   = 10000; // Used for endgame scoring

    Score eval(Board& pos);
    Score evalGameState(Board& pos);
    Score material(const Board& pos);
}