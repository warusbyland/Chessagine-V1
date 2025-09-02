#pragma once

#include <vector>
#include "../Board/Board.h"

enum Direction {
    NORTH_EAST, NORTH, NORTH_WEST, EAST, WEST, SOUTH_EAST, SOUTH, SOUTH_WEST, DIRECTION_NB
};

//               NW                  N                   NE
//                  ---+---+---+---+---+---+---+---+---+
//                   8 | 7 |   |   | 8 |   |   | 9 |   | 
//                  ---+---+---+---+---+---+---+---+---+
//                   7 |   | 7 |   | 8 |   | 9 |   |   |
//                  ---+---+---+---+---+---+---+---+---+
//                   6 |   |   | 7 | 8 | 9 |   |   |   |
//                  ---+---+---+---+---+---+---+---+---+
//                W  5 |-1 |-1 |-1 | % | 1 | 1 | 1 | 1 | E
//                  ---+---+---+---+---+---+---+---+---+
//                   4 |   |   |-9 |-8 |-7 |   |   |   |
//                  ---+---+---+---+---+---+---+---+---+
//                   3 |   |-9 |   |-8 |   |-7 |   |   |
//                  ---+---+---+---+---+---+---+---+---+
//                   2 |-9 |   |   |-8 |   |   |-7 |   |
//                  ---+---+---+---+---+---+---+---+---+
//                   1 |0x0|0x1|0x2| . |   |   |   |-7 |
//                  ---+---+---+---+---+---+---+---+---+
//                     | A | B | C | D | E | F | G | H |
//               SW                  S                   SE

namespace MoveGen {
    void initRays();
    BB attackedBy(Board &pos, Color side);
    BB attackedBy(Board &pos, Color side, BB occupancy);
    Moves genLegalMoves(Board &pos);
    void moveGenDebug(Board &pos);
    void perftDebug(Board &pos, int depth);
}