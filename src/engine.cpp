#include "engine.h"
#include "eval.h"
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
  searchKeyStack.clear();
  searchKeyStack.push_back(position.hash());

  auto all_moves = MoveGen::generate_moves(position);

  if (all_moves.empty()) return Move{};

  // prioritize captures, promotions and checks to improve pruning performance
  std::sort(all_moves.begin(), all_moves.end(), [](const Move& a, const Move& b) {
    const bool aTactical = (a.flags & (CAPTURE | PROMOTION | CHECK)) != 0;
    const bool bTactical = (b.flags & (CAPTURE | PROMOTION | CHECK)) != 0;
    return aTactical > bTactical;
  });

  Move best_move = all_moves.front();
  Move pv_move = best_move;

  for (uint8_t depth = 1; depth <= searchDepth; depth++) {
    auto pv_it = std::find(all_moves.begin(), all_moves.end(), pv_move);
    if (pv_it != all_moves.end() && pv_it != all_moves.begin()) {
      std::iter_swap(all_moves.begin(), pv_it);
    }

    Score alpha = -INF;
    const Score beta = INF;
    Score iteration_best_score = -INF;
    Move iteration_best_move = all_moves.front();

    for (const Move& move : all_moves) {
      auto undo_info = position.do_move(move);
      searchKeyStack.push_back(position.hash());
      Score cur_score = -negamax(position, static_cast<uint8_t>(depth - 1), -beta, -alpha);
      searchKeyStack.pop_back();
      position.undo_move(undo_info);

      if (cur_score > iteration_best_score) {
        iteration_best_move = move;
        iteration_best_score = cur_score;
      }

      alpha = std::max(alpha, cur_score);
    }

    best_move = iteration_best_move;
    pv_move = iteration_best_move;
  }

  return best_move;
}

// Negated Minimax searches for the optimal score in the given position
Score Engine::negamax(Position& pos, uint8_t depth, Score alpha, Score beta) {
  if (is_threefold(pos)) return 0;

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
    searchKeyStack.push_back(pos.hash());
    Score score = -negamax(pos, depth - 1, -beta, -alpha);
    searchKeyStack.pop_back();
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

bool Engine::is_threefold(const Position& pos) const {
  if (searchKeyStack.empty()) return false;

  const Key key = pos.hash();
  int repetitions = 0;

  const int last = static_cast<int>(searchKeyStack.size()) - 1;
  const int maxBack = std::min<int>(static_cast<int>(pos.get_state().rule50), last);
  const int minIdx = last - maxBack;

  for (int i = last; i >= minIdx; i -= 2) {
    if (searchKeyStack[i] == key) {
      repetitions++;
      if (repetitions >= 3) return true;
    }
  }

  return false;
}

Engine::~Engine() {
  printf("\033[1;31mshutting down prometheus.\033[0m\n");
}
