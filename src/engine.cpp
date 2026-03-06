#include "engine.h"

Engine::Engine() {

}

Score Engine::eval() {
  Score score = 0;

  score += material_score();

  return position.get_player() == Color::WHITE ? score : -score;
}

Move Engine::search(uint8_t depth) {
  Move bestMove = {0, 0};

  return bestMove;
}

Score Engine::material_score() {
  Score score = 0;

  score += 100 * (position.count_pieces(W_PAWN) - position.count_pieces(B_PAWN));
  score += 300 * (position.count_pieces(W_KNIGHT) - position.count_pieces(B_KNIGHT));
  score += 320 * (position.count_pieces(W_BISHOP) - position.count_pieces(B_BISHOP));
  score += 500 * (position.count_pieces(W_ROOK) - position.count_pieces(B_ROOK));
  score += 900 * (position.count_pieces(W_QUEEN) - position.count_pieces(B_QUEEN));
  return score;
}

Engine::~Engine() {
  printf("\033[1;31mshutting down prometheus.\033[0m\n");
}