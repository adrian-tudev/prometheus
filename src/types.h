#ifndef TYPES_H
#define TYPES_H

#include <cassert>
#include <cstdint>

using Bitboard = uint64_t;
using Key      = uint64_t;
using Score    = int32_t;
using Square   = uint8_t;

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
  BQ = 1 << 3,

  ALL = WK | WQ | BK | BQ,
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
  PROMOTION = 1 << 5,
  CHECK = 1 << 6,
};

struct Move {
  Square from;
  Square to;
  PieceType promotion = EMPTY;
  PieceType captured_piece = EMPTY;
  MoveFlags flags = static_cast<MoveFlags>(0);
};

inline bool operator==(const Move& a, const Move& b) {
  return a.from == b.from && a.to == b.to && a.promotion == b.promotion;
}

inline bool operator!=(const Move& a, const Move& b) {
  return !(a == b);
}

#endif
