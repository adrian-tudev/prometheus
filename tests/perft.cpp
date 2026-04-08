#include <cstdint>
#include <iostream>

#include "bitboard.h"
#include "movegen.h"
#include "position.h"
#include "utils.h"

using u64 = unsigned long long;

u64 perft(uint8_t depth);

Position pos;

std::string positions[] = {
  STARTING_FEN,
  "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",
  "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 "
};

int main() {
  std::cout << "PERFT tests" << std::endl;
  Bitboards::init();

  int pos_n = 1;
  for (auto position : positions) {
    pos.set(position);
    for (int depth = 0; depth < 6; depth++) {
      std::cout << "position " << pos_n << 
        " depth: " << depth << ": " 
        << perft(depth) << std::endl;
    }
    pos_n++;
  }
}

u64 perft(uint8_t depth) {
  if (depth == 0) return 1LL;
  u64 nodes = 0;

  auto moves = MoveGen::generate_moves(pos);
  for (auto move : moves) {
    auto undo_info = pos.do_move(move);
    nodes += perft(depth - 1);
    pos.undo_move(undo_info);
  }
  return nodes;
}
