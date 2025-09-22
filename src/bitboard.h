#ifndef BITBOARD_H
#define BITBOARD_H

#include <algorithm>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <vector>

#include "types.h"

using std::pair;
using std::map;

namespace Bitboards {
  // global precomputed bitboards
  extern Bitboard movementMasks[pieceTypes][64];
  extern map<pair<Square, Bitboard>, Bitboard> bishopAttack;
  extern map<pair<Square, Bitboard>, Bitboard> rookAttack;

  // rook movement masks
  constexpr Bitboard rankMask = 0xFFULL;
  constexpr Bitboard fileMask = 0x0101010101010101ULL;

  // castling masks
  constexpr Bitboard kingsideCastleMaskW = 0x60ULL;
  constexpr Bitboard queensideCastleMaskW = 0x0EULL;

  constexpr Bitboard kingsideCastleMaskB = kingsideCastleMaskW << (7 * 8);
  constexpr Bitboard queensideCastleMaskB = queensideCastleMaskW << (7 * 8);

  void init();
  Bitboard clear_bit(Bitboard board, int square);
  Bitboard set_bit(Bitboard board, int square);
  void print(Bitboard board);
  void print_raw(Bitboard board);

} // namespace Bitboards

#endif