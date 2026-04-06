#ifndef UI_H
#define UI_H

#include "engine.h"

class UI {
public:
  UI();
  ~UI() = default;

  void loop();

private:
  Engine engine;

  // stores history of positions
  std::vector<Position> positions;

  bool is_own_piece(Move move);
  bool is_move_legal(Move move);
};

#endif
