#include "Eval.h"

using namespace Eval;

Score Eval::material(const Board& pos) {
    Score score = 0;

    for (int piece = 0; piece < 6; piece++) {
        score += __builtin_popcountll(pos.getPieceBoard(static_cast<Piece>(piece)))
         * Piece_Materials[piece];
        score -= __builtin_popcountll(pos.getPieceBoard(static_cast<Piece>(piece + 6)))
         * Piece_Materials[piece];
    }

    return score;
}

// Assuming game is ONGOING
Score Eval::eval(Board& pos) {
    Score score = material(pos);

    // Add positional evaluation here later (e.g., PSTs, mobility)
    
    // return (pos.getTurn() == WHITE) ? score : -score; // return in different perspective
    return score;
}

Score Eval::evalGameState(Board& pos) {
    Color c = pos.getTurn();

    GameState gamestate = pos.gameState();
    if (gamestate == CHECKMATE) return c == WHITE ? -INF : INF;
    return 0; // Stalemate
}