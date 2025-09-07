#include "bitboard.h"

using namespace std;

const uint64_t hor_mask = 0xFFULL;
const uint64_t ver_mask = 0x0101010101010101ULL;

void sliding_pieces();
void nonsliding_pieces();

Bitboard attackMasks[pieceTypes][64];

// precomputed pseudo-legal bitboards for all pieces
Bitboard movementMasks[pieceTypes][64];

namespace Bitboards {
  void init() {
    printf("Computing bitboards...\n");
    sliding_pieces();
    nonsliding_pieces();
    printf("Computed bitboards.\n");
  }

  void print(Bitboard board) {
    string s = bitset<64>(board).to_string();
    for (int i = 0; i < 8; i++) {
      for (int j = 7; j >= 0; j--) {
        cout << s[i * 8 + j];
      }
      cout << endl;
    }
  }

  void print_raw(Bitboard board) {
    string b = bitset<64>(board).to_string();
    cout << b << endl;
  }

  Bitboard set_bit(Bitboard board, int square) {
    return board | (1ULL << square);
  }

  Bitboard clear_bit(Bitboard board, int square) {
    return board & ~(1ULL << square);
  }
} // namespace Bitboards

void sliding_pieces() {
  Bitboard bishopMovement[64];
  Bitboard rookMovement[64];
  for (uint8_t square = 0; square < 64; square++) {
    Bitboard straight = 0;
    Bitboard diagonal = 0;

    int rank = square / 8;
    int file = square % 8;

    straight |= hor_mask << (rank * 8);
    straight |= ver_mask << file;

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
    diagonal = Bitboards::clear_bit(diagonal, square);
    straight = Bitboards::clear_bit(straight, square);

    bishopMovement[square] = diagonal;
    rookMovement[square] = straight;

    movementMasks[B_BISHOP][square] = bishopMovement[square];
    movementMasks[W_BISHOP][square] = movementMasks[B_BISHOP][square];

    movementMasks[B_ROOK][square] = rookMovement[square];
    movementMasks[W_ROOK][square] = movementMasks[B_ROOK][square];

    movementMasks[B_QUEEN][square] = bishopMovement[square] | rookMovement[square];
    movementMasks[W_QUEEN][square] = movementMasks[B_QUEEN][square];
  }
}

void nonsliding_pieces() {
  for (uint8_t square = 0; square < 64; square++) {
    // pawn movement
    movementMasks[W_PAWN][square] = 0;
    movementMasks[B_PAWN][square] = 0;

    // no moves when reaching promotion rank
    if (square / 8 < 7)
      movementMasks[W_PAWN][square] |= 1ULL << (square + 8);
    if (square / 8 == 1)
      movementMasks[W_PAWN][square] |= (1ULL << (square + 16));
    
    if (square / 8 > 0)
      movementMasks[B_PAWN][square] |= 1ULL << (square - 8);
    if (square / 8 == 6)
      movementMasks[B_PAWN][square] |= (1ULL << (square - 16));

    // knight movement
    movementMasks[W_KNIGHT][square] = 0;
    Bitboard knightBox = 0;
    for (uint8_t y = max(square / 8 - 2, 0); y <= min(square / 8 + 2, 7); y++) {
      for (uint8_t x = max(square % 8 - 2, 0); x <= min(square % 8 + 2, 7); x++) {
        knightBox |= (1ULL << (y * 8 + x));
      }
    }
    int8_t knightDirs[] = {-17, -15, -10, -6, 6, 10, 15, 17};
    for (int8_t dir : knightDirs) {
      movementMasks[W_KNIGHT][square] |= knightBox & (1ULL << (square + dir));
    }

    movementMasks[W_KING][square] = 0;
    for (uint8_t y = max(square / 8 - 1, 0); y <= min(square / 8 + 1, 7); y++) {
      for (uint8_t x = max(square % 8 - 1, 0); x <= min(square % 8 + 1, 7); x++) {
        movementMasks[W_KING][square] |= 1ULL << (y * 8 + x);
      }
    }
    movementMasks[B_KNIGHT][square] = movementMasks[W_KNIGHT][square];
    movementMasks[B_KING][square] = movementMasks[W_KING][square];
  }
}