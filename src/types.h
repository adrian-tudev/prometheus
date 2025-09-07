#ifndef TYPES_H
#define TYPES_H

#include <cassert>
#include <cstdint>

using Bitboard = uint64_t;
using Square = uint8_t;

// OBS! EMPTY square not counted
constexpr uint8_t pieceTypes = 12;

enum Color { BLACK, WHITE };

enum PieceType { 
  B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
  W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
  EMPTY
};

struct Move {
  uint8_t target;
  uint8_t dest;
};

inline bool is_white(PieceType type) {
  assert(type != EMPTY);
  return type >= W_PAWN && type <= W_KING;
}

#endif