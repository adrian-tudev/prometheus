#include <iostream>

#include "engine.h"

int main(int argc, char* args[]) {
  std::cout << "\033[32mprometheus v.0\033[0m" << std::endl;
  Engine engine;
  std::cout << "evaluation: " << engine.eval() << std::endl;

  std::cout << "\033[1;31mkilling prometheus.\033[0m" << std::endl;
  return 0;
}
