#ifndef BITBOARD_H
#define BITBOARD_H

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <iostream>

#include "types.h"

// precomputed bitboards
static Bitboard pieceMovement[pieceTypes][64];

namespace Bitboards {

  void init();
  void sliding_pieces();
  void nonsliding_pieces();

  Bitboard clear_bit(Bitboard board, int square);
  Bitboard set_bit(Bitboard board, int square);
  void print(Bitboard board);
  void print_raw(Bitboard board);

} // namespace Bitboards

#endif