#ifndef BITBOARD_H
#define BITBOARD_H

#include <algorithm>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "types.h"

using std::unordered_map;

namespace Bitboards {
  struct AttackKey {
    Square square;
    Bitboard blockers;

    bool operator==(const AttackKey&) const noexcept = default;
  };

  struct AttackKeyHash {
    size_t operator()(const AttackKey& key) const noexcept {
      size_t h1 = std::hash<Square>{}(key.square);
      size_t h2 = std::hash<Bitboard>{}(key.blockers);
      return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    }
  };

  // global precomputed bitboards
  extern Bitboard movementMasks[PIECE_TYPES][64];
  extern unordered_map<AttackKey, Bitboard, AttackKeyHash> bishopAttack;
  extern unordered_map<AttackKey, Bitboard, AttackKeyHash> rookAttack;

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
