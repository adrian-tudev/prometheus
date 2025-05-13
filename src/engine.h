#ifndef ENGINE_H
#define ENGINE_H

#include <cstdint>

#include "movegen.h"
#include "position.h"

class Engine {
  public:
    Engine();
    ~Engine() = default;

    int32_t eval();
    Move search(int depth);

  private:
    Position position;
    State gameState;
    MoveGen moveGenerator;

    int32_t material_score();
};

#endif
