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

    int32_t eval();
    Move search(uint8_t depth);

  private:
    Position position;
    MoveGen moveGenerator;

    int32_t material_score();
};

#endif
