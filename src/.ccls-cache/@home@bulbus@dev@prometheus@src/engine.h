#ifndef ENGINE_H
#define ENGINE_H

#include <cstdint>

#include "position.h"

class Engine {
  public:
    Engine();
    ~Engine() = default;

    int32_t eval();
    void search();

  private:
    Position position;
    State gameState;

    int32_t material_score();
};

#endif
