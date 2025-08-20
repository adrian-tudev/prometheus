#include <iostream>
#include "engine.h"

int main(int argc, char* args[]) {
  if (argc > 1 && std::string(args[1]) == "-h") {
    std::cout << "usage: prometheus [-h] [-depth depth]" << std::endl;
    return 1;
  }
  std::cout << "\033[32mprometheus v.0\033[0m" << std::endl;

  Engine engine;

  /*
  std::string command;
  while (std::getline(std::cin, command)) {
    if (command == "quit") break;
    std::cout << "you said: " << command << std::endl;
  }*/
  std::cout << "evaluation: " << engine.eval() << std::endl;
  return 0;
}
