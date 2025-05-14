#include "movegen.h"

using namespace Bitboards;

const uint64_t hor_mask = 0xFFULL;
const uint64_t ver_mask = 0x0101010101010101ULL;

MoveGen::MoveGen() {
  // precompute pseudo-legal moves for all pieces
  compute_sliding_pieces();
  compute_nonsliding_pieces();
}

void MoveGen::compute_nonsliding_pieces() {
  for (uint8_t square = 0; square < 64; square++) {
    // pawn movement
    pawnMovement[0][square] = 0;
    pawnMovement[1][square] = 0;

    // no moves when reaching promotion rank
    pawnMovement[0][square] |= 1ULL << (square + 8);
    if (square / 8 == 1)
      pawnMovement[0][square] |= (1ULL << (square + 16));
    
    if (square / 8 > 0)
      pawnMovement[1][square] |= 1ULL << (square - 8);
    if (square / 8 == 6)
      pawnMovement[1][square] |= (1ULL << (square - 16));

    // knight movement
    knightMovement[square] = 0;
    Bitboard knightBox = 0;
    for (uint8_t y = std::max(square / 8 - 2, 0); y <= std::min(square / 8 + 2, 7); y++) {
      for (uint8_t x = std::max(square % 8 - 2, 0); x <= std::min(square % 8 + 2, 7); x++) {
        knightBox |= (1ULL << (y * 8 + x));
      }
    }
    int8_t knightDirs[] = {-17, -15, -10, -6, 6, 10, 15, 17};
    for (int8_t dir : knightDirs) {
      knightMovement[square] |= knightBox & (1ULL << (square + dir));
    }

    kingMovement[square] = 0;
    for (uint8_t y = std::max(square / 8 - 1, 0); y <= std::min(square / 8 + 1, 7); y++) {
      for (uint8_t x = std::max(square % 8 - 1, 0); x <= std::min(square % 8 + 1, 7); x++) {
        kingMovement[square] |= 1ULL << (y * 8 + x);
      }
    }
  }
}

void MoveGen::compute_sliding_pieces() {
  for (uint8_t square = 0; square < 64; square++) {
    Bitboard straight = 0;
    Bitboard diagonal = 0;

    straight |= hor_mask << ((square / 8) * 8);
    straight |= ver_mask << (square % 8);

    int8_t ne = 9, sw = -9;
    int8_t nw = 7, se = -7;

    int8_t cnt = 0;
    while (square + cnt < 64) {
      diagonal |= (1ULL << (square + cnt));
      if ((square + cnt) % 8 == 7) break;
      cnt += ne;
    }

    cnt = 0;
    while (square + cnt < 64) {
      diagonal |= (1ULL << (square + cnt));
      if ((square + cnt) % 8 == 0) break;
      cnt += nw;
    }

    cnt = 0;
    while (square + cnt >= 0) {
      diagonal |= (1ULL << (square + cnt));
      if ((square + cnt) % 8 == 7) break;
      cnt += se;
    }

    cnt = 0;
    while (square + cnt >= 0) {
      diagonal |= (1ULL << (square + cnt));
      if ((square + cnt) % 8 == 0) break;
      cnt += sw;
    }
    
    // current square should not be included in the movement
    diagonal = clear_bit(diagonal, square);
    straight = clear_bit(straight, square);

    bishopMovement[square] = diagonal;
    rookMovement[square] = straight;
    queenMovement[square] = straight | diagonal;
  }
}
