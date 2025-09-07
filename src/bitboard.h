#ifndef BITBOARD_H
#define BITBOARD_H

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <iostream>

#include "types.h"

extern Bitboard attackMasks[pieceTypes][64];

namespace Bitboards {

  void init();

  Bitboard clear_bit(Bitboard board, int square);
  Bitboard set_bit(Bitboard board, int square);
  void print(Bitboard board);
  void print_raw(Bitboard board);

} // namespace Bitboards

#endif