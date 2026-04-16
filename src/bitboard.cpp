#include "bitboard.h"

namespace Bitboards {

// precomputed pseudo-legal bitboards for all pieces
Bitboard movementMasks[PIECE_TYPES][64];
unordered_map<AttackKey, Bitboard, AttackKeyHash> rookAttack;
unordered_map<AttackKey, Bitboard, AttackKeyHash> bishopAttack;

// forward declaring
void sliding_pieces();
void nonsliding_pieces();
void blockers();
std::vector<Bitboard> gen_blockers(Bitboard movementMask);
Bitboard legal_rook_squares(Square sq, Bitboard blockers);
Bitboard legal_bishop_squares(Square sq, Bitboard blockers);

void init() {
  printf("Computing bitboards...\n");
  auto start = std::chrono::high_resolution_clock::now();

  sliding_pieces();
  nonsliding_pieces();
  blockers();

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  long long ms = (long long) duration.count();
  printf("Bitboards computed in %lld ms.\n", ms);
}

// print bitboard in 8x8 format
void print(Bitboard board) {
  std::string s = std::bitset<64>(board).to_string();
  for (int i = 0; i < 8; i++) {
    for (int j = 7; j >= 0; j--) {
      printf("%c", s[i * 8 + j]);
    }
    printf("\n");
  }
  printf("\n");
}

// print raw 64 bit binary representation
void print_raw(Bitboard board) {
  std::string b = std::bitset<64>(board).to_string();
  printf("%s", b.c_str());
}

Bitboard set_bit(Bitboard board, int square) {
  return board | (1ULL << square);
}

Bitboard clear_bit(Bitboard board, int square) {
  return board & ~(1ULL << square);
}

void sliding_pieces() {
  Bitboard bishopMovement[64];
  Bitboard rookMovement[64];
  for (uint8_t square = 0; square < 64; square++) {
    Bitboard straight = 0;
    Bitboard diagonal = 0;

    int rank = square / 8;
    int file = square % 8;

    straight |= rankMask << (rank * 8);
    straight |= fileMask << file;

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
    
    // no moves when reaching promotion rank
    if (square / 8 > 0)
      movementMasks[B_PAWN][square] |= 1ULL << (square - 8);
    if (square / 8 == 6)
      movementMasks[B_PAWN][square] |= (1ULL << (square - 16));

    // knight movement
    movementMasks[W_KNIGHT][square] = 0;
    Bitboard knightBox = 0;
    for (uint8_t y = std::max(square / 8 - 2, 0); y <= std::min(square / 8 + 2, 7); y++) {
      for (uint8_t x = std::max(square % 8 - 2, 0); x <= std::min(square % 8 + 2, 7); x++) {
        knightBox |= (1ULL << (y * 8 + x));
      }
    }
    int8_t knightDirs[] = {-17, -15, -10, -6, 6, 10, 15, 17};
    for (int8_t dir : knightDirs) {
      int target = (int) square + dir;
      if (target < 0 || target >= 64) continue;
      movementMasks[W_KNIGHT][square] |= knightBox & (1ULL << target);
    }

    movementMasks[W_KING][square] = 0;
    for (uint8_t y = std::max(square / 8 - 1, 0); y <= std::min(square / 8 + 1, 7); y++) {
      for (uint8_t x = std::max(square % 8 - 1, 0); x <= std::min(square % 8 + 1, 7); x++) {
        movementMasks[W_KING][square] |= 1ULL << (y * 8 + x);
      }
    }
    movementMasks[B_KNIGHT][square] = movementMasks[W_KNIGHT][square];
    movementMasks[B_KING][square] = movementMasks[W_KING][square];
  }
}

// Given a movementMask add 1s into the places of the blockers
std::vector<Bitboard> gen_blockers(Bitboard movementMask) {
  std::vector<int> idc;
  for (int i = 0; i < 64; i++) {
    if (movementMask & (1ULL << i)) {
      idc.push_back(i);
    }
  }
  int possibleSquares = __builtin_popcountll(movementMask);
  // number of possible blockers
  int subsets = 1 << possibleSquares;
  std::vector<Bitboard> blockers(subsets, 0);
  for (int pattern = 0; pattern < subsets; pattern++) {
    for(int i = 0; i < idc.size(); i++) {
      if (pattern & (1ULL << i)) {
        blockers[pattern] |= (1ULL << idc[i]);
      }
    }
  }
  return blockers;
}

// given a blocker configuration, return all legal moves for a rook on sq
Bitboard legal_rook_squares(Square sq, Bitboard blockers) {
  Bitboard legalMoves = 0;

  int rank = sq / 8;
  int file = sq % 8;

  // right
  for (int f = file + 1; f < 8; f++) {
    legalMoves |= (1ULL << (rank * 8 + f));
    if (blockers & (1ULL << (rank * 8 + f))) break;
  }
  // left
  for (int f = file - 1; f >= 0; f--) {
    legalMoves |= (1ULL << (rank * 8 + f));
    if (blockers & (1ULL << (rank * 8 + f))) break;
  }
  // up
  for (int r = rank + 1; r < 8; r++) {
    legalMoves |= (1ULL << (r * 8 + file));
    if (blockers & (1ULL << (r * 8 + file))) break;
  }
  // down
  for (int r = rank - 1; r >= 0; r--) {
    legalMoves |= (1ULL << (r * 8 + file));
    if (blockers & (1ULL << (r * 8 + file))) break;
  }

  return legalMoves;
}

Bitboard legal_bishop_squares(Square sq, Bitboard blockers) {
  Bitboard legalMoves = 0;

  int rank = sq / 8;
  int file = sq % 8;

  // northeast
  for (int r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++) {
    legalMoves |= (1ULL << (r * 8 + f));
    if (blockers & (1ULL << (r * 8 + f))) break;
  }
  // northwest
  for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--) {
    legalMoves |= (1ULL << (r * 8 + f));
    if (blockers & (1ULL << (r * 8 + f))) break;
  }
  // southeast
  for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++) {
    legalMoves |= (1ULL << (r * 8 + f));
    if (blockers & (1ULL << (r * 8 + f))) break;
  }
  // southwest
  for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
    legalMoves |= (1ULL << (r * 8 + f));
    if (blockers & (1ULL << (r * 8 + f))) break;
  }

  return legalMoves;
}

// precompute blockers
void blockers() {
  for (Square sq = 0; sq < 64; sq++) {
    std::vector<Bitboard> rook_blockers = gen_blockers(movementMasks[W_ROOK][sq]);
    for (Bitboard pattern : rook_blockers) {
      Bitboard legalMoves = legal_rook_squares(sq, pattern);
      rookAttack[AttackKey{sq, pattern}] = legalMoves;
    }

    std::vector<Bitboard> bishop_blockers = gen_blockers(movementMasks[W_BISHOP][sq]);
    for (Bitboard pattern : bishop_blockers) {
      Bitboard legalMoves = legal_bishop_squares(sq, pattern);
      bishopAttack[AttackKey{sq, pattern}] = legalMoves;
    }
  }
}

} // namespace Bitboards
