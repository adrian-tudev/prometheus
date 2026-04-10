#include "engine.h"
#include "movegen.h"

Engine::Engine() {
}

Score Engine::eval() {
  return eval(position);
}

void Engine::set_position(const Position &pos) { position = pos; }

Move Engine::ponder() {
  Move best_move;
  Score best_score = -INF;
  auto all_moves = MoveGen::generate_moves(position);

  if (all_moves.empty()) return Move{};

  for (auto move : all_moves) {
    auto undo_info = position.do_move(move);
    Score cur_score = -negamax(position, SEARCH_DEPTH);
    position.undo_move(undo_info);

    if (cur_score > best_score) {
      best_move = move;
      best_score = cur_score;
    }
  }

  return best_move;
}

// Negated Minimax searches for the optimal score in the given position
Score Engine::negamax(Position pos, uint8_t depth) {
  if (depth == 0) return eval(pos);

  Score best = -INF;
  auto all_moves = MoveGen::generate_moves(pos);
  if (all_moves.empty()) return eval(pos);

  for (auto move : all_moves) {
    auto undo_info = pos.do_move(move);
    best = std::max(-negamax(pos, depth - 1), best);
    pos.undo_move(undo_info);
  }

  return best;
}

Score Engine::eval(const Position& pos) const {
  Score score = 0;

  score += material_score(pos);

  return pos.get_player() == Color::WHITE ? score : -score;
}

Score Engine::material_score(const Position& pos) const {
  Score score = 0;

  score += 100 * (pos.count_pieces(W_PAWN) - pos.count_pieces(B_PAWN));
  score += 300 * (pos.count_pieces(W_KNIGHT) - pos.count_pieces(B_KNIGHT));
  score += 320 * (pos.count_pieces(W_BISHOP) - pos.count_pieces(B_BISHOP));
  score += 500 * (pos.count_pieces(W_ROOK) - pos.count_pieces(B_ROOK));
  score += 900 * (pos.count_pieces(W_QUEEN) - pos.count_pieces(B_QUEEN));
  return score;
}


Score Engine::material_score() {
  return material_score(position);
}

Engine::~Engine() {
  printf("\033[1;31mshutting down prometheus.\033[0m\n");
}
