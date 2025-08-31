#include "Chessagine.h"

using namespace Eval;

Score Chessagine::minimax(Board& pos, int depth, bool maximizingPlayer, Score alpha, Score beta) {
    if (depth == 0) return eval(pos);

    Moves ml = MoveGen::genLegalMoves(pos);
    if (ml.empty()) return evalGameState(pos);

    if (maximizingPlayer) {
        Score maxEval = -INF;
        for (Move m : ml) {
            pos.move(m);
            Score eval = minimax(pos, depth - 1, false, alpha, beta);
            pos.undo();
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        Score minEval = INF;
        for (Move m : ml) {
            pos.move(m);
            Score eval = minimax(pos, depth - 1, true, alpha, beta);
            pos.undo();
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

Move Chessagine::think(Board& pos, int depth) {
    Color turn = pos.getTurn();

    Score bestScore = turn == WHITE ? -INF : INF;
    Move bestMove;
    Score alpha = -INF;
    Score beta = INF;

    Moves ml = MoveGen::genLegalMoves(pos);
    if (ml.empty()) {
        pos.gameState();
        return 0 ; 
    }

    for (Move m : ml) {
        pos.move(m);
        Score eval = minimax(pos, depth - 1, !turn, alpha, beta);
        pos.undo();
        if ((turn == WHITE && bestScore < eval) || (turn == BLACK && bestScore > eval)) {
            bestScore = eval;
            bestMove = m;
        }
    }
    
    return bestMove;
}
