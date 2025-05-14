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
    if (square / 8 < 7)
      pawnMovement[0][square] |= 1ULL << (square + 8);
    
    if (square / 8 > 0)
      pawnMovement[1][square] |= 1ULL << (square - 8);

    if (square / 8 == 1) {
      pawnMovement[0][square] |= (1ULL << (square + 16));
    }

    // knight movement

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
