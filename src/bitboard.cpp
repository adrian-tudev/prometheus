#include "bitboard.h"

using namespace std;

const uint64_t hor_mask = 0xFFULL;
const uint64_t ver_mask = 0x0101010101010101ULL;

namespace Bitboards {
  void init() {
    sliding_pieces();
    nonsliding_pieces();
  }

  void sliding_pieces() {
    Bitboard bishopMovement[64];
    Bitboard rookMovement[64];
    Bitboard queenMovement[64];
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

      pieceMovement[B_BISHOP][square] = bishopMovement[square];
      pieceMovement[W_BISHOP][square] = pieceMovement[B_BISHOP][square];

      pieceMovement[B_ROOK][square] = rookMovement[square];
      pieceMovement[W_ROOK][square] = pieceMovement[B_ROOK][square];

      pieceMovement[B_QUEEN][square] = queenMovement[square];
      pieceMovement[W_QUEEN][square] = pieceMovement[B_QUEEN][square];
    }
  }

  void nonsliding_pieces() {
    Bitboard pawnMovement[2][64];
    Bitboard knightMovement[64];
    Bitboard kingMovement[64];
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
      for (uint8_t y = max(square / 8 - 2, 0); y <= min(square / 8 + 2, 7); y++) {
        for (uint8_t x = max(square % 8 - 2, 0); x <= min(square % 8 + 2, 7); x++) {
          knightBox |= (1ULL << (y * 8 + x));
        }
      }
      int8_t knightDirs[] = {-17, -15, -10, -6, 6, 10, 15, 17};
      for (int8_t dir : knightDirs) {
        knightMovement[square] |= knightBox & (1ULL << (square + dir));
      }

      kingMovement[square] = 0;
      for (uint8_t y = max(square / 8 - 1, 0); y <= min(square / 8 + 1, 7); y++) {
        for (uint8_t x = max(square % 8 - 1, 0); x <= min(square % 8 + 1, 7); x++) {
          kingMovement[square] |= 1ULL << (y * 8 + x);
        }
      }
      pieceMovement[W_PAWN][square] = pawnMovement[0][square];
      pieceMovement[B_PAWN][square] = pawnMovement[1][square];

      pieceMovement[W_KING][square] = kingMovement[square];
      pieceMovement[B_KING][square] = kingMovement[square];

      pieceMovement[W_KNIGHT][square] = knightMovement[square];
      pieceMovement[B_KNIGHT][square] = knightMovement[square];
    }
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