#ifndef ENGINE_H
#define ENGINE_H

#include <cstdint>

#include "movegen.h"
#include "position.h"
#include "utils.h"

class Engine {
  public:
    Engine();
    ~Engine();

    int32_t eval();
    Move search(uint8_t depth);

  private:
    State gameState;
    Position position;
    MoveGen moveGenerator;

    int32_t material_score();
};

#endif
