#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <cstdint>
#include <vector>

#include "bitboard.h"
#include "position.h"
#include "types.h"

class MoveGen {
  public:
    MoveGen();
    ~MoveGen() = default;

    std::vector<Move> generate_moves(PieceType piece, Square sq, const Position& pos);
  private:
};

#endif