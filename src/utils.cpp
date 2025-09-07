#include "utils.h"

std::string format(Square sq) {
  std::string files = "abcdefgh";
  std::string ranks = "12345678";
  return std::string() + files[sq % 8] + ranks[sq / 8];
}