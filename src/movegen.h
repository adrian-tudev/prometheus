#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <vector>

#include "position.h"
#include "types.h"

using std::vector;

namespace MoveGen {

// all moves for current player
vector<Move> generate_moves(const Position& pos);

// all moves for a specific piece
vector<Move> generate_moves_at(Square sq, const Position& pos);

// returns bitboard of all squares attacked by the given player
Bitboard attack_mask(const Position& pos, Color player);

} // namespace MoveGen

#endif
