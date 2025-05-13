#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <cstdint>
#include <vector>

#include "bitboard.h"
#include "position.h"

// in bitboard fashion
using Square = uint64_t;

struct Move {
  Square target;
  Square dest;
};

/*
 *  TODO
 *
 * - precomputed movement bitboards
 * 
 */

class MoveGen {
  public:
    MoveGen();
    ~MoveGen() = default;

  private:
    Bitboard pawnMovement[2][64];
    Bitboard knightMovement[64];
    void compute_nonsliding_pieces();

    Bitboard bishopMovement[64];
    Bitboard rookMovement[64];
    Bitboard queenMovement[64];

    void compute_sliding_pieces();

};

#endif
