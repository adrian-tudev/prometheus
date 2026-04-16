#include "utils.h"

string format(Move move) {
  std::string res = format(move.from) + format(move.to);
  if (move.promotion != EMPTY) {
    char promoChar = charToPiece[move.promotion];
    res += tolower(promoChar);
  }
  return res;
}

string format(Square sq) {
  return string() + files[sq % 8] + ranks[sq / 8];
}

optional<Move> parse_move(const string& str) {
  if (str.length() < 4 || str.length() > 5) return std::nullopt;
  Move move;
  move.from = (str[0] - 'a') + (str[1] - '1') * 8;
  move.to = (str[2] - 'a') + (str[3] - '1') * 8;
  if (str.length() == 5) {
    char promoChar = str[4];
    auto it = std::find(charToPiece.begin(), charToPiece.end(), promoChar);
    if (it == charToPiece.end()) return std::nullopt;
    move.promotion = (PieceType) std::distance(charToPiece.begin(), it);
  }
  return move;
}

uint64_t splitmix64(uint64_t& seed) {
    uint64_t z = (seed += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

bool is_sliding(PieceType piece) {
  return piece == W_BISHOP || piece == B_BISHOP || piece == W_ROOK || piece == B_ROOK || piece == W_QUEEN || piece == B_QUEEN;
}

Color get_color(PieceType piece) {
  return piece >= W_PAWN ? WHITE : BLACK;
}
