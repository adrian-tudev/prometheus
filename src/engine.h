#ifndef ENGINE_H
#define ENGINE_H

#include "position.h"

#define PROMETHEUS_VERSION "0.1"

class Engine {
public:
  Engine();
  ~Engine();

  Score eval(const Position& pos) const;
  Move ponder();
  void set_position(const Position &pos);

  static const Score INF = 1e9;
  static const unsigned int SEARCH_DEPTH = 5;

private:
  Position position;

  Score negamax(Position pos, uint8_t depth, Score alpha, Score beta);
  Score material_score(const Position& pos) const;
};

#endif
