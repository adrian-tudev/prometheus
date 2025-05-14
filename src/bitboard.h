#ifndef BITBOARD_H
#define BITBOARD_H

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <iostream>

using Bitboard = uint64_t;

namespace Bitboards {

  Bitboard set_bit(Bitboard board, uint8_t index);
  Bitboard clear_bit(Bitboard board, uint8_t index);

  void print(Bitboard board);
  void print_raw(Bitboard board);

} // namespace Bitboards

#endif