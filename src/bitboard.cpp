#include "bitboard.h"

using namespace std;

namespace Bitboards {
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

  Bitboard set_bit(Bitboard board, uint8_t index) {
    return board | (1ULL << index);
  }

  Bitboard clear_bit(Bitboard board, uint8_t index) {
    return board & ~(1ULL << index);
  }
} // namespace Bitboards