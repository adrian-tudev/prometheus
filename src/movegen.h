#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <cstdint>
#include <vector>

#include "bitboard.h"
#include "position.h"
#include "types.h"
#include "utils.h"

class MoveGen {
  public:
    MoveGen() = default;
    ~MoveGen() = default;

    // all moves for current player
    std::vector<Move> generate_moves(const Position& pos);

    // all moves for a specific piece
    std::vector<Move> generate_moves_at(Square sq, const Position& pos) const;
};

#endif