#pragma once

#include "Board/Board.h"
#include "Evaluation/Eval.h"
#include "MoveGen/moveGen.h"
#include "moves.h"

namespace Chessagine {
    Score minimax(Board& pos, int depth, bool maximizingPlayer, Score alpha, Score beta);
    Move think(Board &pos, int depth);
}