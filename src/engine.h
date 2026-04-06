#ifndef ENGINE_H
#define ENGINE_H

#include "movegen.h"
#include "position.h"
#include "utils.h"

#define PROMETHEUS_VERSION "0.1"

class Engine {
public:
  Engine();
  ~Engine();

  Score eval();
  Move search(uint8_t depth);
  void set_position(const Position& pos) { position = pos; }

private:
  Position position;

  Score material_score();
};

#endif
