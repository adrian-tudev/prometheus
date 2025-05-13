#include "bitboard.h"

using namespace std;
void Bitboards::print(Bitboard board) {
  string s = bitset<64>(board).to_string();
  for (int i = 0; i < 8; i++) {
    for (int j = 7; j >= 0; j--) {
      cout << s[i * 8 + j];
    }
    cout << endl;
  }
}

void Bitboards::print_raw(Bitboard board) {
  string b = bitset<64>(board).to_string();
  cout << b << endl;
}