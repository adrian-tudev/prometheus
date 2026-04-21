#ifndef ENGINE_H
#define ENGINE_H

#include "position.h"
#include "tt.h"

#include <vector>

#define PROMETHEUS_VERSION "0.1"

class Engine {
public:
  Engine();
  ~Engine();

  Move ponder();
  void set_position(const Position &pos);
  void set_search_depth(uint8_t depth);
  uint8_t get_search_depth() const;

  static const Score INF = 1e9;
  static const uint8_t DEFAULT_SEARCH_DEPTH = 6;
  static const uint8_t MIN_SEARCH_DEPTH = 1;
  static const uint8_t MAX_SEARCH_DEPTH = 10;

private:
  Position position;
  uint8_t searchDepth = DEFAULT_SEARCH_DEPTH;
  TT tt{128};
  std::vector<Key> searchKeyStack;

  Score negamax(Position& pos, uint8_t depth, Score alpha, Score beta);
  bool is_threefold(const Position& pos) const;
};

#endif
