#ifndef TYPES_H
#define TYPES_H

#include <cassert>
#include <cstdint>

using Bitboard = uint64_t;
using Square = uint8_t;

// OBS! EMPTY square not counted
constexpr uint8_t pieceTypes = 12;

enum Color { BLACK, WHITE };

// Castling rights bitmask
enum CastlingRights {
  WHITE_KING_SIDE = 1,
  WHITE_QUEEN_SIDE = 2,
  BLACK_KING_SIDE = 4,
  BLACK_QUEEN_SIDE = 8
};

enum PieceType { 
  B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
  W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
  EMPTY
};

inline bool piece_is_white(PieceType type) {
  assert(type != EMPTY);
  return type >= W_PAWN && type <= W_KING;
}

struct Move {
  Square from;
  Square to;
  PieceType promotion = EMPTY;
};

inline bool operator==(const Move& a, const Move& b) {
  return a.from == b.from && a.to == b.to && a.promotion == b.promotion;
}

inline bool operator!=(const Move& a, const Move& b) {
  return !(a == b);
}

#endif