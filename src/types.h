#ifndef TYPES_H
#define TYPES_H

#include <cassert>
#include <cstdint>

using Bitboard = uint64_t;
using Square = uint8_t;
using Score = int32_t;

// OBS! EMPTY square not counted
constexpr uint8_t pieceTypes = 12;

enum Color { BLACK, WHITE };

enum PieceType { 
  B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
  W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
  EMPTY
};

enum CastlingRights {
  WK = 1 << 0,
  WQ = 1 << 1,
  BK = 1 << 2,
  BQ = 1 << 3
};

inline bool piece_is_white(PieceType type) {
  assert(type != EMPTY);
  return type >= W_PAWN && type <= W_KING;
}

enum MoveFlags {
  CAPTURE = 1 << 0,
  DOUBLE_PAWN_PUSH = 1 << 1,
  KING_CASTLE = 1 << 2,
  QUEEN_CASTLE = 1 << 3,
  EN_PASSANT = 1 << 4,
  PROMOTION = 1 << 5
};

struct Move {
  Square from;
  Square to;
  MoveFlags flags = static_cast<MoveFlags>(0);
  PieceType promotion = EMPTY;
};

inline bool operator==(const Move& a, const Move& b) {
  return a.from == b.from && a.to == b.to && a.promotion == b.promotion;
}

inline bool operator!=(const Move& a, const Move& b) {
  return !(a == b);
}

#endif