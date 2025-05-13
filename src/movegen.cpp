#include "movegen.h"

const uint64_t hor_mask = 0xFFULL;
const uint64_t ver_mask = 0x0101010101010101ULL;

MoveGen::MoveGen() {
  compute_sliding_pieces();
  compute_nonsliding_pieces();
}

void MoveGen::compute_nonsliding_pieces() {

}

void MoveGen::compute_sliding_pieces() {
  for (uint8_t i = 0; i < 64; i++) {
    Bitboard straight = 0;
    Bitboard diagonal = 0;

    straight |= hor_mask << ((i / 8) * 8);
    straight |= ver_mask << (i % 8);

    int8_t ne = 9, sw = -9;
    int8_t nw = 7, se = -7;

    int8_t cnt = 0;
    while (i + cnt < 64) {
      diagonal |= (1ULL << (i + cnt));
      if ((i + cnt) % 8 == 7) break;
      cnt += ne;
    }

    cnt = 0;
    while (i + cnt < 64) {
      diagonal |= (1ULL << (i + cnt));
      if ((i + cnt) % 8 == 0) break;
      cnt += nw;
    }

    cnt = 0;
    while (i + cnt >= 0) {
      diagonal |= (1ULL << (i + cnt));
      if ((i + cnt) % 8 == 7) break;
      cnt += se;
    }

    cnt = 0;
    while (i + cnt >= 0) {
      diagonal |= (1ULL << (i + cnt));
      if ((i + cnt) % 8 == 0) break;
      cnt += sw;
    }

    bishopMovement[i] = diagonal;
    rookMovement[i] = straight;
    queenMovement[i] = straight | diagonal;
  }
}
