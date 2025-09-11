#include "utils.h"

std::string format(Square sq) {
  std::string files = "abcdefgh";
  std::string ranks = "12345678";
  return std::string() + files[sq % 8] + ranks[sq / 8];
}

bool is_sliding(PieceType piece) {
  return piece == W_BISHOP || piece == B_BISHOP || piece == W_ROOK || piece == B_ROOK || piece == W_QUEEN || piece == B_QUEEN;
}