#include "engine.h"
#include "movegen.h"

#include <algorithm>

Engine::Engine() {
}

void Engine::set_position(const Position &pos) { position = pos; }

void Engine::set_search_depth(uint8_t depth) {
  if (depth < MIN_SEARCH_DEPTH) {
    searchDepth = MIN_SEARCH_DEPTH;
    return;
  }
  if (depth > MAX_SEARCH_DEPTH) {
    searchDepth = MAX_SEARCH_DEPTH;
    return;
  }
  searchDepth = depth;
}

uint8_t Engine::get_search_depth() const {
  return searchDepth;
}

Move Engine::ponder() {
  tt.new_search();

  Move best_move;
  Score best_score = -INF;
  Score alpha = -INF;
  const Score beta = INF;
  auto all_moves = MoveGen::generate_moves(position);

  if (all_moves.empty()) return Move{};

  // prioritize captures, promotions and checks to improve pruning performance
  std::sort(all_moves.begin(), all_moves.end(), [](const Move& a, const Move& b) {
    const bool aTactical = (a.flags & (CAPTURE | PROMOTION | CHECK)) != 0;
    const bool bTactical = (b.flags & (CAPTURE | PROMOTION | CHECK)) != 0;
    return aTactical > bTactical;
  });

  const uint8_t next_depth = searchDepth > 0 ? searchDepth - 1 : 0;

  for (const Move& move : all_moves) {
    auto undo_info = position.do_move(move);
    Score cur_score = -negamax(position, next_depth, -beta, -alpha);
    position.undo_move(undo_info);

    if (cur_score > best_score) {
      best_move = move;
      best_score = cur_score;
    }

    alpha = std::max(alpha, cur_score);
  }

  return best_move;
}

// Negated Minimax searches for the optimal score in the given position
Score Engine::negamax(Position& pos, uint8_t depth, Score alpha, Score beta) {
  const Score alpha_orig = alpha;
  const Score beta_orig = beta;
  const Key key = pos.hash();

  TTMove ttMove = TT_MOVE_NONE;
  const TTEntry* entry = tt.probe(key);
  if (entry) {
    ttMove = entry->move;
    if (entry->depth >= depth) {
      if (entry->flag == TT_EXACT) return entry->score;
      if (entry->flag == TT_LOWER) alpha = std::max(alpha, entry->score);
      if (entry->flag == TT_HIGHER) beta = std::min(beta, entry->score);
      if (alpha >= beta) return entry->score;
    }
  }

  if (depth == 0) return eval(pos);

  auto all_moves = MoveGen::generate_moves(pos);
  if (all_moves.empty()) {
    if (pos.is_check()) return -INF + 1;
    return 0;
  }

  std::sort(all_moves.begin(), all_moves.end(), [](const Move& a, const Move& b) {
    const bool aTactical = (a.flags & (CAPTURE | PROMOTION | CHECK)) != 0;
    const bool bTactical = (b.flags & (CAPTURE | PROMOTION | CHECK)) != 0;
    return aTactical > bTactical;
  });

  if (ttMove != TT_MOVE_NONE) {
    const Move ttDecoded = unpack_tt_move(ttMove);
    auto it = std::find(all_moves.begin(), all_moves.end(), ttDecoded);
    if (it != all_moves.end() && it != all_moves.begin()) {
      std::iter_swap(all_moves.begin(), it);
    }
  }

  Score best = -INF;
  Move best_move = all_moves.front();

  for (const Move& move : all_moves) {
    auto undo_info = pos.do_move(move);
    Score score = -negamax(pos, depth - 1, -beta, -alpha);
    pos.undo_move(undo_info);

    if (score > best) {
      best = score;
      best_move = move;
    }

    alpha = std::max(alpha, score);
    if (alpha >= beta) break;
  }

  TTFlag flag = TT_EXACT;
  if (best <= alpha_orig) {
    flag = TT_HIGHER;
  } else if (best >= beta_orig) {
    flag = TT_LOWER;
  }

  tt.store(key, best, depth, flag, pack_tt_move(best_move));

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

Engine::~Engine() {
  printf("\033[1;31mshutting down prometheus.\033[0m\n");
}
