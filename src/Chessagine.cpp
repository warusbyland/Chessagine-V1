#include "Chessagine.h"

using namespace Eval;

Score Chessagine::minimax(Board& pos, int depth, bool maximizingPlayer, Score alpha, Score beta) {
    if (depth == 0) return eval(pos);

    Moves ml = MoveGen::genLegalMoves(pos);
    if (ml.empty()) return evalGameState(pos);

    if (maximizingPlayer) {
        Score maxEval = -INF;
        for (Move m : ml) {

            // std::cout << moveFormat(m) << " " << pos.getFen() << "\n";

            pos.move(m);
            // assert(pos.getTurn() == WHITE || pos.getTurn() == BLACK);
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
            // std::cout << moveFormat(m) << " " << pos.getFen() << "\n";

            pos.move(m);
            // assert(pos.getTurn() == WHITE || pos.getTurn() == BLACK);
            Score eval = minimax(pos, depth - 1, true, alpha, beta);
            pos.undo();

            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

void Chessagine::think(Board& pos, int depth) {
    Color c = pos.getTurn();
    bool maximizingPlayer = c == WHITE;

    Score bestScore = c == WHITE ? -INF - 1 : INF + 1; // +1 to garantee a move even if its all checkmates
    Move bestMove;
    Score alpha = -INF;
    Score beta = INF;

    Moves ml = MoveGen::genLegalMoves(pos);
    if (ml.empty()) return;

    for (Move m : ml) {
        pos.move(m);
        Score eval = minimax(pos, depth - 1, !maximizingPlayer, alpha, beta); //prblem
        
        // const int from = fromSquare(m);
        // const int to = toSquare(m);

        // char fromFile = 'a' + (from % 8);
        // int fromRank = 1 + (from / 8);
        // char toFile = 'a' + (to % 8);
        // int toRank = 1 + (to / 8);

        // std::cout << fromFile << fromRank << toFile << toRank << ": " << eval << " eval\n";
        pos.undo();
        if ((maximizingPlayer && bestScore < eval) || (!maximizingPlayer && bestScore > eval)) {
            bestScore = eval;
            bestMove = m;
        }
    }

    const int from = fromSquare(bestMove);
    const int to = toSquare(bestMove);

    char fromFile = 'a' + (from % 8);
    int fromRank = 1 + (from / 8);
    char toFile = 'a' + (to % 8);
    int toRank = 1 + (to / 8);

    std::cout << "Best Move: " << fromFile << fromRank << toFile << toRank << ": " << bestScore << " eval\n\n";
    pos.move(bestMove);
}

