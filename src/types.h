#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

using Bitboard = uint64_t;
using Square = uint64_t;

struct Move {
  Square target;
  Square dest;
};

#endif