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
  "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ",
  "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
  "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ",
};

// results of perft tests at depth 5
u64 results[] = {
  4865609,
  193690690,
  674624,
  15833292,
  89941194,
};

int main() {
  std::cout << "PERFT tests" << std::endl;
  Bitboards::init();

  int pos_n = 1;
  for (int i = 0; i < positions->length(); i++) {
    pos.set(positions[i]);
    std::cout << "Position: " << pos_n++ << std::endl;
    u64 res;
    for (int depth = 0; depth < 6; depth++) {
      res = perft(depth);
      std::cout << "  depth: " << depth << ": " << res << std::endl;
    }
    std::cout << (res == results[i] ? "PASS" : "FAIL") << std::endl;
    std::cout << std::endl;
  }
  std::cout << "Completed tests." << std::endl;
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
