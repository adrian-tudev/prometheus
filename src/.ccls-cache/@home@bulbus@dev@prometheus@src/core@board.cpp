#include "board.h"

using namespace Game;

Board::Board(const std::string& FEN) : m_FEN(FEN) {

}

uint32_t Board::eval() {

  return -1;
}
