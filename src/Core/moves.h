#pragma once

#include <cstdint> 
#include <vector>
#include <iostream>
#include <string>

//                  ---+---+---+---+---+---+---+---+---+
//                   8 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 
//                  ---+---+---+---+---+---+---+---+---+
//                   7 | 8 | 9 | 10| 11| 12| 13| 14| 15|
//                  ---+---+---+---+---+---+---+---+---+
//                   6 | 16| 17| 18| 19| 20| 21| 22| 23|
//                  ---+---+---+---+---+---+---+---+---+
//                   5 | 24| 25| 26| 27| 28| 29| 30| 31| 
//                  ---+---+---+---+---+---+---+---+---+
//                   4 | 32| 33| 34| 35| 36| 37| 38| 39|
//                  ---+---+---+---+---+---+---+---+---+
//                   3 | 40| 41| 42| 43| 44| 45| 46| 47|
//                  ---+---+---+---+---+---+---+---+---+
//                   2 | 48| 49| 50| 51| 52| 53| 54| 55|
//                  ---+---+---+---+---+---+---+---+---+
//                   1 | 56| 57| 58| 59| 60| 61| 62| 63|
//                  ---+---+---+---+---+---+---+---+---+
//                     | A | B | C | D | E | F | G | H |

//                  ---+---+---+---+---+---+---+---+---+
//                   8 | 63| 62| 61| 60| 59| 58| 57| 56| 
//                  ---+---+---+---+---+---+---+---+---+
//                   7 | 55| 54| 53| 52| 51| 50| 49| 48|
//                  ---+---+---+---+---+---+---+---+---+
//                   6 | 47| 46| 45| 44| 43| 42| 41| 40|
//                  ---+---+---+---+---+---+---+---+---+
//                   5 | 39| 38| 37| 36| 35| 34| 33| 32| 
//                  ---+---+---+---+---+---+---+---+---+
//                   4 | 31| 30| 29| 28| 27| 26| 25| 24|
//                  ---+---+---+---+---+---+---+---+---+
//                   3 | 23| 22| 21| 20| 19| 18| 17| 16|
//                  ---+---+---+---+---+---+---+---+---+
//                   2 | 15| 14| 13| 12| 11| 10| 9 | 8 |
//                  ---+---+---+---+---+---+---+---+---+
//                   1 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
//                  ---+---+---+---+---+---+---+---+---+
//                     | A | B | C | D | E | F | G | H |

typedef uint16_t Move;
using BB = uint64_t;
using Moves = std::vector<Move>;

enum Piece {
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,

    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,

    PIECE_NB
};

enum Color : uint8_t { WHITE, BLACK, BOTH, COLOR_NB};
enum GameState { ONGOING, CHECKMATE, STALEMATE };

inline Color operator!(Color c) {
    return Color(1 - c); // Flip color
}

inline Piece getPawn(Color c)   { return c == WHITE ? WHITE_PAWN   : BLACK_PAWN;  }
inline Piece getKnight(Color c) { return c == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT; }
inline Piece getBishop(Color c) { return c == WHITE ? WHITE_BISHOP : BLACK_BISHOP; }
inline Piece getRook(Color c)   { return c == WHITE ? WHITE_ROOK   : BLACK_ROOK;  }
inline Piece getQueen(Color c)  { return c == WHITE ? WHITE_QUEEN  : BLACK_QUEEN; }
inline Piece getKing(Color c)   { return c == WHITE ? WHITE_KING   : BLACK_KING;  }

enum CastlingRights {
    WHITE_KINGSIDE  = 1 << 0, // 0001
    WHITE_QUEENSIDE = 1 << 1, // 0010
    BLACK_KINGSIDE  = 1 << 2, // 0100
    BLACK_QUEENSIDE = 1 << 3, // 1000
};
const uint8_t ROOKCASTLELOSS[64] = {
    /* Squares 0-63 */
    WHITE_QUEENSIDE, 0, 0, 0, 0, 0, 0, WHITE_KINGSIDE,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    BLACK_QUEENSIDE, 0, 0, 0, 0, 0, 0, BLACK_KINGSIDE // rank 8
};
enum Flag {
    QUIET, DOUBLE_PUSH, KING_CASTLE, QUEEN_CASTLE, CAPTURE, EN_PASSANT, 
    PROMO_N,         PROMO_B,         PROMO_R,         PROMO_Q,
    PROMO_CAPTURE_N, PROMO_CAPTURE_B, PROMO_CAPTURE_R, PROMO_CAPTURE_Q
};
namespace BUtils {

    // Single squares
    constexpr BB A1 = 1ULL << 0;
    constexpr BB H8 = 1ULL << 63;

    // Files
    constexpr BB FILE_A = 0x0101010101010101ULL;
    constexpr BB FILE_B = FILE_A << 1;
    constexpr BB FILE_C = FILE_A << 2;
    constexpr BB FILE_D = FILE_A << 3;
    constexpr BB FILE_E = FILE_A << 4;
    constexpr BB FILE_F = FILE_A << 5;
    constexpr BB FILE_G = FILE_A << 6;
    constexpr BB FILE_H = FILE_A << 7;

    constexpr BB NOT_A  = ~FILE_A; // For pawns
    constexpr BB NOT_H  = ~FILE_H;
    constexpr BB NOT_AB  = NOT_A & ~FILE_B; // For knights
    constexpr BB NOT_GH  = NOT_H & ~FILE_G;

    // Ranks
    constexpr BB RANK_1 = 0xFFULL;
    constexpr BB RANK_2 = RANK_1 << (8 * 1);
    constexpr BB RANK_3 = RANK_1 << (8 * 2);
    constexpr BB RANK_4 = RANK_1 << (8 * 3);
    constexpr BB RANK_5 = RANK_1 << (8 * 4);
    constexpr BB RANK_6 = RANK_1 << (8 * 5);
    constexpr BB RANK_7 = RANK_1 << (8 * 6);
    constexpr BB RANK_8 = RANK_1 << (8 * 7);

    // Board areas
    constexpr BB ALL =           0xFFFFFFFFFFFFFFFFULL;
    constexpr BB LIGHT_SQUARES = 0x55AA55AA55AA55AAULL;
    constexpr BB DARK_SQUARES  = ~LIGHT_SQUARES;

    constexpr BB CENTER = (1ULL << 27) | (1ULL << 28) | (1ULL << 35) | (1ULL << 36);
    constexpr BB EXTENDED_CENTER = CENTER |
        (1ULL << 18) | (1ULL << 19) | (1ULL << 20) | (1ULL << 21) |
        (1ULL << 26) | (1ULL << 29) |
        (1ULL << 34) | (1ULL << 37) |
        (1ULL << 42) | (1ULL << 43) | (1ULL << 44) | (1ULL << 45);
}
enum Square {
   H1, G1, F1, E1, D1, C1, B1, A1,
   H2, G2, F2, E2, D2, C2, B2, A2,
   H3, G3, F3, E3, D3, C3, B3, A3,
   H4, G4, F4, E4, D4, C4, B4, A4,
   H5, G5, F5, E5, D5, C5, B5, A5,
   H6, G6, F6, E6, D6, C6, B6, A6,
   H7, G7, F7, E7, D7, C7, B7, A7,
   H8, G8, F8, E8, D8, C8, B8, A8
};

inline Move makeMove(int from, int to, int flag) { return (flag << 12) | (to << 6) | from; }

inline int fromSquare(Move m) { return m & 0x3F; }           // 6 bits
inline int toSquare(Move m)   { return (m >> 6) & 0x3F; }    // next 6 bits
inline Flag moveFlag(Move m)   { return static_cast<Flag>((m >> 12) & 0xF); }    // 4 bits

inline int popLSB(BB &bb) {
    int i = __builtin_ctzll(bb);
    bb &= bb - 1;
    return i;
}
inline BB bs(BB bb, int shift) { return shift > 0 ? bb << shift : bb >> -shift; }
inline BB bm(int bit) { return 1ULL << bit; }

// Visualize Binary
void printBB(BB bb);