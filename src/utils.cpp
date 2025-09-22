#include "utils.h"

std::string format(Move move) {
  std::string res = format(move.from) + format(move.to);
  if (move.promotion != EMPTY) {
    char promoChar = charToPiece[move.promotion];
    res += tolower(promoChar);
  }
  return res;
}

std::string format(Square sq) {
  return std::string() + files[sq % 8] + ranks[sq / 8];
}

Move parse_move(const std::string& str) {
  assert(str.length() >= 4);
  Move move;
  move.from = (str[0] - 'a') + (str[1] - '1') * 8;
  move.to = (str[2] - 'a') + (str[3] - '1') * 8;
  if (str.length() == 5) {
    char promoChar = str[4];
    auto it = std::find(charToPiece.begin(), charToPiece.end(), promoChar);
    assert(it != charToPiece.end());
    move.promotion = (PieceType) std::distance(charToPiece.begin(), it);
  }
  return move;
}

bool is_sliding(PieceType piece) {
  return piece == W_BISHOP || piece == B_BISHOP || piece == W_ROOK || piece == B_ROOK || piece == W_QUEEN || piece == B_QUEEN;
}

Color get_color(PieceType piece) {
  return piece >= W_PAWN ? WHITE : BLACK;
}