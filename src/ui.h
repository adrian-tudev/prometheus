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
    Position position;

    // for user input validation
    MoveGen movegen;
};

#endif