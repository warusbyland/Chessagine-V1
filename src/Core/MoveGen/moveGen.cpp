#include "moveGen.h"
#include "../moves.h"
#include <chrono>

constexpr BB KING_MOVES[64] = {
    770ULL,1797ULL,3594ULL,7188ULL,14376ULL,28752ULL,57504ULL,49216ULL,197123ULL,460039ULL,920078ULL,
    1840156ULL,3680312ULL,7360624ULL,14721248ULL,12599488ULL,50463488ULL,117769984ULL,235539968ULL,
    471079936ULL,942159872ULL,1884319744ULL,3768639488ULL,3225468928ULL,12918652928ULL,30149115904ULL,
    60298231808ULL,120596463616ULL,241192927232ULL,482385854464ULL,964771708928ULL,825720045568ULL,
    3307175149568ULL,7718173671424ULL,15436347342848ULL,30872694685696ULL,61745389371392ULL,123490778742784ULL,
    246981557485568ULL,211384331665408ULL,846636838289408ULL,1975852459884544ULL,
    3951704919769088ULL,7903409839538176ULL,15806819679076352ULL,31613639358152704ULL,63227278716305408ULL,
    54114388906344448ULL,216739030602088448ULL,505818229730443264ULL,1011636459460886528ULL,2023272918921773056ULL,
    4046545837843546112ULL,8093091675687092224ULL,16186183351374184448ULL,13853283560024178688ULL,
    144959613005987840ULL,362258295026614272ULL,724516590053228544ULL,1449033180106457088ULL,
    2898066360212914176ULL,5796132720425828352ULL,11592265440851656704ULL,4665729213955833856ULL
};
constexpr BB KNIGHT_MOVES[64] = {
132096ULL, 329728ULL, 659712ULL, 1319424ULL, 2638848ULL, 5277696ULL, 10489856ULL, 4202496ULL, 
33816580ULL, 84410376ULL, 168886289ULL, 337772578ULL, 675545156ULL, 1351090312ULL, 2685403152ULL, 
1075839008ULL, 8657044482ULL, 21609056261ULL, 43234889994ULL, 86469779988ULL, 172939559976ULL, 
345879119952ULL, 687463207072ULL, 275414786112ULL, 2216203387392ULL, 5531918402816ULL, 
11068131838464ULL, 22136263676928ULL, 44272527353856ULL, 88545054707712ULL, 175990581010432ULL, 
70506185244672ULL, 567348067172352ULL, 1416171111120896ULL, 2833441750646784ULL, 5666883501293568ULL, 
11333767002587136ULL, 22667534005174272ULL, 45053588738670592ULL, 18049583422636032ULL, 
145241105196122112ULL, 362539804446949376ULL, 725361088165576704ULL, 1450722176331153408ULL, 
2901444352662306816ULL, 5802888705324613632ULL, 11533718717099671552ULL, 4620693356194824192ULL, 
288234782788157440ULL, 576469569871282176ULL, 1224997833292120064ULL, 2449995666584240128ULL, 
4899991333168480256ULL, 9799982666336960512ULL, 1152939783987658752ULL, 2305878468463689728ULL, 
1128098930098176ULL, 2257297371824128ULL, 4796069720358912ULL, 9592139440717824ULL, 
19184278881435648ULL, 38368557762871296ULL, 4679521487814656ULL, 9077567998918656ULL
}; 

alignas(64) BB RAYS[DIRECTION_NB][64];

bool is_valid(int from, int to, int dir) {
    int from_file = from % 8;
    int to_file = to % 8;
    int from_rank = from / 8;
    int to_rank = to / 8;

    switch (dir) {
        case NORTH:        return to_rank > from_rank;
        case SOUTH:        return to_rank < from_rank;
        case EAST:         return to_file > from_file;
        case WEST:         return to_file < from_file;
        case NORTH_EAST:   return to_rank > from_rank && to_file > from_file;
        case NORTH_WEST:   return to_rank > from_rank && to_file < from_file;
        case SOUTH_EAST:   return to_rank < from_rank && to_file > from_file;
        case SOUTH_WEST:   return to_rank < from_rank && to_file < from_file;
    }
    return false;
}
void MoveGen::initRays() {
    int directionOffsets[8] = { +9, +8, +7, +1, -1, -7, -8, -9 };

    for (int sq = 0; sq < 64; ++sq) {
        for (int dir = 0; dir < DIRECTION_NB; ++dir) {
            BB ray = 0ULL;
            int offset = directionOffsets[dir];
            int next = sq + offset;

            while (next >= 0 && next < 64 && is_valid(sq, next, dir)) {
                ray |= (1ULL << next);
                next += offset;
            }

            RAYS[dir][sq] = ray;
        }
    }
}

inline BB knightAttacks(int sq) { return KNIGHT_MOVES[sq]; }
inline BB kingAttacks(int sq) { return KING_MOVES[sq]; }
BB slidingAttacks(int sq, BB occupancy, Direction dir) {
    BB ray = RAYS[dir][sq];
    BB blockers = ray & occupancy;

    if (blockers == 0)    
        return ray; 
  
    if (dir < 4) {
        int blocker_sq = __builtin_ctzll(blockers); // closest in forward direction
        return ray & ~(RAYS[dir][blocker_sq]);

    } else {
        int blocker_sq = 63 - __builtin_clzll(blockers); // closest in reverse direction
        return ray & ~(RAYS[dir][blocker_sq]);
    }
}
inline BB bishopAttacks(int sq, BB occupancy) {
    return slidingAttacks(sq, occupancy, NORTH_EAST) |
           slidingAttacks(sq, occupancy, SOUTH_EAST) |
           slidingAttacks(sq, occupancy, SOUTH_WEST) |
           slidingAttacks(sq, occupancy, NORTH_WEST);
}
inline BB rookAttacks(int sq, BB occupancy) {
    return slidingAttacks(sq, occupancy, NORTH) |
           slidingAttacks(sq, occupancy, EAST) |
           slidingAttacks(sq, occupancy, SOUTH) |
           slidingAttacks(sq, occupancy, WEST);
}
inline BB queenAttacks(int sq, BB occupancy) {
    return bishopAttacks(sq, occupancy) | rookAttacks(sq, occupancy);
}

inline BB pawnsAttacks(Color color, BB pawns) {
    return (color == WHITE)
        ? ((pawns << 7) & BUtils::NOT_H) | ((pawns << 9) & BUtils::NOT_A)
        : ((pawns >> 9) & BUtils::NOT_H) | ((pawns >> 7) & BUtils::NOT_A);
}
inline BB knightsAttacks(BB knights) {
    return (((knights << 15) | (knights >> 17)) & BUtils::NOT_H) | 
    (((knights << 6) | (knights >> 10)) & BUtils::NOT_GH) | 
    (((knights << 17) | (knights >> 15)) & BUtils::NOT_A) | 
    (((knights << 10) | (knights >> 6)) & BUtils::NOT_AB);
}
inline BB kingsAttacks(BB kings) {
    return (kings << 8) | (kings >> 8) |                          
            (((kings << 1) | (kings << 9) | (kings >> 7)) & BUtils::NOT_A) |
            (((kings >> 1) | (kings << 7) | (kings >> 9)) & BUtils::NOT_H);
}
BB bishopsAttacks(BB bishops, BB occupancy) {
    BB atk = 0;
    while (bishops) {
        int sq = popLSB(bishops);
        atk |= bishopAttacks(sq, occupancy);
    }
    return atk;
}
BB rooksAttacks(BB rooks, BB occupancy) {
    BB atk = 0;
    while (rooks) {
        int sq = popLSB(rooks);
        atk |= rookAttacks(sq, occupancy);
    }
    return atk;
}
BB queensAttacks(BB queens, BB occupancy) {
    return bishopsAttacks(queens, occupancy) |
           rooksAttacks(queens, occupancy);
}
BB MoveGen::attackedBy(Board &pos, Color side) {
    BB occupancy = pos.getAllOccupancy() & ~pos.getKingBB(!side); // make "us" king invisible
    return pawnsAttacks(side, pos.getPawnBB(side)) |
           knightsAttacks(pos.getKnightBB(side)) |
           bishopsAttacks(pos.getBishopBB(side), occupancy) |
           rooksAttacks(pos.getRookBB(side), occupancy) |
           queensAttacks(pos.getQueenBB(side), occupancy) |
           kingsAttacks(pos.getKingBB(side));
}
BB MoveGen::attackedBy(Board &pos, Color side, BB occupancy) {
    return pawnsAttacks(side, pos.getPawnBB(side)) |
           knightsAttacks(pos.getKnightBB(side)) |
           bishopsAttacks(pos.getBishopBB(side), occupancy) |
           rooksAttacks(pos.getRookBB(side), occupancy) |
           queensAttacks(pos.getQueenBB(side), occupancy) |
           kingsAttacks(pos.getKingBB(side));
}

void genKingMoves(Board &pos, Moves &moves) {
    const Color us = pos.getTurn();
    const Color them = !us;

    BB kings = pos.getKingBB(us);
    const BB occupancy = pos.getAllOccupancy();
    const BB illegal = ~pos.getOccupancy(us) & ~pos.getAttackedBy(them);


    while (kings) {
        int from = popLSB(kings);
        BB atk = kingAttacks(from) & illegal;  // Remove friendly squares & walking into danger

        while (atk) {
            int to = popLSB(atk);
            moves.push_back(makeMove(from, to, occupancy & bm(to) ? CAPTURE : QUIET));
        }
    }

    uint8_t rights = pos.castleRights();

    if (us == WHITE) {
        // White Kingside: E1 -> G1, empty: F1, G1; not attacked: E1, F1, G1; rook on H1
        if ((rights & WHITE_KINGSIDE) &&
            !(pos.getOccupancy(BOTH) & ((1ULL << F1) | (1ULL << G1))) &&
            !(pos.getAttackedBy(BLACK) & ((1ULL << E1) | (1ULL << F1) | (1ULL << G1))) &&
            (pos.getRookBB(WHITE) & (1ULL << H1)))
        {
            moves.push_back(makeMove(E1, G1, KING_CASTLE));
        }

        // White Queenside: E1 -> C1, empty: B1, C1, D1; not attacked: C1, D1, E1; rook on A1
        if ((rights & WHITE_QUEENSIDE) &&
            !(pos.getOccupancy(BOTH) & ((1ULL << B1) | (1ULL << C1) | (1ULL << D1))) &&
            !(pos.getAttackedBy(BLACK) & ((1ULL << C1) | (1ULL << D1) | (1ULL << E1))) &&
            (pos.getRookBB(WHITE) & (1ULL << A1)))
        {
            moves.push_back(makeMove(E1, C1, QUEEN_CASTLE));
        }

    } else {
        // Black Kingside: E8 -> G8, empty: F8, G8; not attacked: E8, F8, G8; rook on H8
        if ((rights & BLACK_KINGSIDE) &&
            !(pos.getOccupancy(BOTH) & ((1ULL << F8) | (1ULL << G8))) &&
            !(pos.getAttackedBy(WHITE) & ((1ULL << E8) | (1ULL << F8) | (1ULL << G8))) &&
            (pos.getRookBB(BLACK) & (1ULL << H8)))
        {
            moves.push_back(makeMove(E8, G8, KING_CASTLE));
        }

        // Black Queenside: E8 -> C8, empty: B8, C8, D8; not attacked: C8, D8, E8; rook on A8
        if ((rights & BLACK_QUEENSIDE) &&
            !(pos.getOccupancy(BOTH) & ((1ULL << B8) | (1ULL << C8) | (1ULL << D8))) &&
            !(pos.getAttackedBy(WHITE) & ((1ULL << C8) | (1ULL << D8) | (1ULL << E8))) &&
            (pos.getRookBB(BLACK) & (1ULL << A8)))
        {
            moves.push_back(makeMove(E8, C8, QUEEN_CASTLE));
        }
    }
}
void genKnightMoves(Board &pos, Moves &moves) {
    const Color us = pos.getTurn();
    const BB illegal = ~pos.getOccupancy(us);
    const BB occupancy = pos.getAllOccupancy();

    BB knights = pos.getKnightBB(us);

    while (knights) {
        int from = popLSB(knights);
        BB atk = knightAttacks(from) & illegal;

        while (atk) {
            int to = popLSB(atk);
            if (!pos.isMoveSafe(us, from, to)) continue;
            moves.push_back(makeMove(from, to, occupancy & bm(to) ? CAPTURE : QUIET));
        }
    }
}
void genBishopMoves(Board &pos, Moves &moves) {
    const Color us = pos.getTurn();
    const BB illegal = ~pos.getOccupancy(us);
    const BB occupancy = pos.getAllOccupancy();

    BB bishops = pos.getBishopBB(us);

    while (bishops) {
        int from = popLSB(bishops);
        BB atk = bishopAttacks(from, occupancy) & illegal;

        while (atk) {
            int to = popLSB(atk);
            if (!pos.isMoveSafe(us, from, to)) continue;
            moves.push_back(makeMove(from, to, occupancy & bm(to) ? CAPTURE : QUIET));
        }
    }
}
void genRookMoves(Board &pos, Moves &moves) {
    const Color us = pos.getTurn();
    const BB illegal = ~pos.getOccupancy(us);
    const BB occupancy = pos.getAllOccupancy();

    BB rooks = pos.getRookBB(us);

    while (rooks) {
        int from = popLSB(rooks);
        BB atk = rookAttacks(from, occupancy) & illegal;

        while (atk) {
            int to = popLSB(atk);
            if (!pos.isMoveSafe(us, from, to)) continue;
            moves.push_back(makeMove(from, to, occupancy & bm(to) ? CAPTURE : QUIET));
        }
    }
}
void genQueenMoves(Board &pos, Moves &moves) {
    const Color us = pos.getTurn();
    const BB illegal = ~pos.getOccupancy(us);
    const BB occupancy = pos.getAllOccupancy();

    BB queens = pos.getQueenBB(us);

    while (queens) {
        int from = popLSB(queens);
        BB atk = queenAttacks(from, occupancy) & illegal;

        while (atk) {
            int to = popLSB(atk);
            if (!pos.isMoveSafe(us, from, to)) continue;
            moves.push_back(makeMove(from, to, occupancy & bm(to) ? CAPTURE : QUIET));
        }
    }
}
void genPawnMoves(Board &pos, Moves &moves) {
    const Color us = pos.getTurn();
    const Color them = !us;

    const BB pawns = pos.getPawnBB(us);
    const BB illegal = ~pos.getOccupancy(us);
    const BB enemy = pos.getOccupancy(them);
    const BB occupancy = pos.getAllOccupancy();

    const int dir = us == WHITE ? 8 : -8;
    const BB rank_3 = us == WHITE ? BUtils::RANK_3 : BUtils::RANK_6;
    const int dirEast = us == WHITE ? 9 : -7;
    const int dirWest = us == WHITE ? 7 : -9;
    const BB promoRank = us == WHITE ? BUtils::RANK_8 : BUtils::RANK_1;

    // QUIET moves
    BB shift = bs(pawns, dir) & ~occupancy;
    BB doubleshift = bs(shift & rank_3, dir) & ~occupancy;

    BB promo = shift & promoRank; // Extract QUIET promotions
    shift &= ~promoRank; // Remove promotion squares

    // CAPTURE moves
    BB atkEast = bs(pawns, dirEast) & BUtils::NOT_A & enemy;
    BB atkWest = bs(pawns, dirWest) & BUtils::NOT_H & enemy;

    BB promoEast = atkEast & promoRank; // Extract CAPTURE promotions
    BB promoWest = atkWest & promoRank;
    atkEast &= ~promoRank; //Remove promotion squares
    atkWest &= ~promoRank;

    
    while (shift) {
        int to = popLSB(shift);
        int from = to - dir;
        if (!pos.isMoveSafe(us, from, to)) continue;
        moves.push_back(makeMove(from, to, QUIET)); 
    }
    while (doubleshift) {
        int to = popLSB(doubleshift);
        int from = to - dir * 2;
        if (!pos.isMoveSafe(us, from, to)) continue;
        moves.push_back(makeMove(from, to, DOUBLE_PUSH));
    }
    while (atkEast) {
        int to = popLSB(atkEast);
        int from = to - dirEast;
        if (!pos.isMoveSafe(us, from, to)) continue;
        moves.push_back(makeMove(from, to, CAPTURE));
    }
    while (atkWest) {
        int to = popLSB(atkWest);
        int from = to - dirWest;
        if (!pos.isMoveSafe(us, from, to)) continue;
        moves.push_back(makeMove(from, to, CAPTURE));
    }
    while (promo) {
        int to = popLSB(promo);
        int from = to - dir;
        if (!pos.isMoveSafe(us, from, to)) continue;
        moves.push_back(makeMove(from, to, PROMO_Q)); 
        moves.push_back(makeMove(from, to, PROMO_R)); 
        moves.push_back(makeMove(from, to, PROMO_B)); 
        moves.push_back(makeMove(from, to, PROMO_N)); 
    }
    while (promoEast) {
        int to = popLSB(promoEast);
        int from = to - dirEast;
        if (!pos.isMoveSafe(us, from, to)) continue;
        moves.push_back(makeMove(from, to, PROMO_CAPTURE_Q)); 
        moves.push_back(makeMove(from, to, PROMO_CAPTURE_R)); 
        moves.push_back(makeMove(from, to, PROMO_CAPTURE_B)); 
        moves.push_back(makeMove(from, to, PROMO_CAPTURE_N)); 
    }
    while (promoWest) {
        int to = popLSB(promoWest);
        int from = to - dirWest;
        if (!pos.isMoveSafe(us, from, to)) continue;
        moves.push_back(makeMove(from, to, PROMO_CAPTURE_Q)); 
        moves.push_back(makeMove(from, to, PROMO_CAPTURE_R)); 
        moves.push_back(makeMove(from, to, PROMO_CAPTURE_B)); 
        moves.push_back(makeMove(from, to, PROMO_CAPTURE_N)); 
    }
    // Enpassant
    uint8_t epSq = pos.getEnpassantSq();
    if (epSq != -1) {
        BB attackers;
        if (us == WHITE)
            attackers = ((bm(epSq - 9) & ~BUtils::FILE_H) |
                        (bm(epSq - 7) & ~BUtils::FILE_A)) & pawns;
        else
            attackers = ((bm(epSq + 7) & ~BUtils::FILE_H) |
                        (bm(epSq + 9) & ~BUtils::FILE_A)) & pawns;

        while (attackers) {
            int from = popLSB(attackers);
            moves.push_back(makeMove(from, epSq, EN_PASSANT));
        }
    }
}

Moves MoveGen::genLegalMoves(Board &pos) {
    Moves moveList;
    moveList.reserve(100);

    genPawnMoves(pos, moveList);
    genKnightMoves(pos, moveList);
    genBishopMoves(pos, moveList);
    genRookMoves(pos, moveList);
    genQueenMoves(pos, moveList);
    genKingMoves(pos, moveList); 

    return moveList;
}

int perft(Board &pos, int depth) {
    if (depth == 0) return 1;

    Moves ml = MoveGen::genLegalMoves(pos);
    if (ml.empty()) return 0;
    
    int nodes = 0;
    for (Move m : ml) {
        pos.move(m);
        nodes += perft(pos, depth - 1);
        pos.undo();
    }

    return nodes;
}
void MoveGen::perftDebug(Board &pos, int depth) {
    std::string files[8] = { "a", "b", "c", "d", "e", "f", "g", "h" };

    auto start = std::chrono::high_resolution_clock::now();

    int totalNodes = 0;
    Moves ml = MoveGen::genLegalMoves(pos);
    if (ml.empty()) return;

    for (Move m : ml) {
        pos.move(m);
        const int nodes = perft(pos, depth - 1);
        totalNodes += nodes;
        const int from = fromSquare(m);
        const int to = toSquare(m);

        std::cout << files[from % 8] << (from / 8 + 1) << 
                     files[to % 8] << (to / 8 + 1) << ": " << nodes << "\n";

        pos.undo();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "\nNodes searched: " << totalNodes << " in " << duration.count() << 
                " microseconds (" << static_cast<float>(totalNodes) / duration.count() << " mn/s)\n";
}

