#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

using Bitboard = uint64_t;
using Square = uint64_t;

// OBS! EMPTY not counted
constexpr uint8_t pieceTypes = 12;

enum PieceType { 
  B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
  W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
  EMPTY
};

struct Move {
  Square target;
  Square dest;
};

#endif