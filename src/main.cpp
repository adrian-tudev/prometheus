#include <cstdint>

#include "engine.h"
#include "ui.h"

/*

  Self-contained chess engine
  - position representation
  - move generation
  - search
  - evaluation
  - interface

*/

int main(int argc, char* args[]) {
  if (argc > 1 && std::string(args[1]) == "-h") {
    printf("usage: prometheus [-h] [-depth depth]\n");
    return 1;
  }
  UI ui;
  ui.loop();

  return 0;
}
