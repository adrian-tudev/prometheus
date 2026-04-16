#include "eval.h"

namespace {

Score material_score(const Position& pos);

};

Score eval(const Position& pos) {
  Score score = 0;

  score += material_score(pos);

  return pos.get_player() == Color::WHITE ? score : -score;
}

namespace {

Score material_score(const Position& pos) {
  Score score = 0;

  score += 100 * (pos.count_pieces(W_PAWN) - pos.count_pieces(B_PAWN));
  score += 300 * (pos.count_pieces(W_KNIGHT) - pos.count_pieces(B_KNIGHT));
  score += 320 * (pos.count_pieces(W_BISHOP) - pos.count_pieces(B_BISHOP));
  score += 500 * (pos.count_pieces(W_ROOK) - pos.count_pieces(B_ROOK));
  score += 900 * (pos.count_pieces(W_QUEEN) - pos.count_pieces(B_QUEEN));
  return score;
}

}


