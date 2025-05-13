#ifndef BITBOARD_H
#define BITBOARD_H

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <iostream>

using Bitboard = uint64_t;

namespace Bitboards {

  void print(Bitboard board);
  void print_raw(Bitboard board);

} // namespace Bitboards

#endif
