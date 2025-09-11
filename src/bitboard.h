#ifndef BITBOARD_H
#define BITBOARD_H

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <map>
#include <vector>

#include "types.h"

extern Bitboard movementMasks[pieceTypes][64];
extern std::map<std::pair<Square, Bitboard>, Bitboard> rookAttack;
extern std::map<std::pair<Square, Bitboard>, Bitboard> bishopAttack;
namespace Bitboards {

  void init();

  // converts the blocker pattern to a key for the attack map
  int pattern_to_key(Bitboard occupied, Bitboard movementMask);

  Bitboard clear_bit(Bitboard board, int square);
  Bitboard set_bit(Bitboard board, int square);
  void print(Bitboard board);
  void print_raw(Bitboard board);

} // namespace Bitboards

#endif