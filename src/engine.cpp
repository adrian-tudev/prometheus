#include "engine.h"

const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

// checkmate
constexpr int32_t MAX_EVAL = 1e5;

Engine::Engine() {
  position.set(STARTING_FEN);
  // position.print();
}

int32_t Engine::eval() {
  int32_t score = 0;
  
  score += material_score();

  return gameState.white ? score : -score;
}

int32_t Engine::material_score() {
  int32_t score = 0;

  score += 100 * (position.count_pieces(W_PAWN) - position.count_pieces(B_PAWN));
  score += 300 * (position.count_pieces(W_KNIGHT) - position.count_pieces(B_KNIGHT));
  score += 350 * (position.count_pieces(W_BISHOP) - position.count_pieces(B_BISHOP));
  score += 500 * (position.count_pieces(W_ROOK) - position.count_pieces(B_ROOK));
  score += 900 * (position.count_pieces(W_QUEEN) - position.count_pieces(B_QUEEN));
  return score;
}

Engine::~Engine() {
  std::cout << "\033[1;31mshutting down prometheus.\033[0m" << std::endl;
}