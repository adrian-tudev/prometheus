#include "eval.h"

#include <array>

namespace {

Score material_score(const Position& pos);
Score piece_square_score(const Position& pos);

constexpr Square mirror_square(Square sq) {
  return static_cast<Square>(sq ^ 56);
}

constexpr bool is_white_piece(PieceType piece) {
  return piece >= W_PAWN && piece <= W_KING;
}

constexpr int piece_class_index(PieceType piece) {
  switch (piece) {
    case W_PAWN:
    case B_PAWN:
      return 0;
    case W_KNIGHT:
    case B_KNIGHT:
      return 1;
    case W_BISHOP:
    case B_BISHOP:
      return 2;
    case W_ROOK:
    case B_ROOK:
      return 3;
    case W_QUEEN:
    case B_QUEEN:
      return 4;
    case W_KING:
    case B_KING:
      return 5;
    case EMPTY:
      return -1;
  }
  return -1;
}

using PstTable = std::array<Score, 64>;
using PstSet = std::array<PstTable, 6>;

// Tables are indexed from white's perspective (a1..h8).
constexpr PstSet kPieceSquareTables = {{
  // Pawn
  {{
     0,   0,   0,   0,   0,   0,   0,   0,
     8,  10,  10,  12,  12,  10,  10,   8,
     6,   8,  10,  16,  16,  10,   8,   6,
     4,   6,   8,  20,  20,   8,   6,   4,
     3,   5,   7,  14,  14,   7,   5,   3,
     2,   4,   6,   8,   8,   6,   4,   2,
     4,   6,   6, -10, -10,   6,   6,   4,
     0,   0,   0,   0,   0,   0,   0,   0,
  }},
  // Knight
  {{
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -30,   5,  10,  15,  15,  10,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50,
  }},
  // Bishop
  {{
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -10,  10,  10,  12,  12,  10,  10, -10,
    -10,   0,  12,  15,  15,  12,   0, -10,
    -10,   5,  10,  15,  15,  10,   5, -10,
    -10,   0,   8,  10,  10,   8,   0, -10,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20,
  }},
  // Rook
  {{
     0,   0,   5,   8,   8,   5,   0,   0,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
     5,  10,  10,  10,  10,  10,  10,   5,
     0,   0,   0,   2,   2,   0,   0,   0,
  }},
  // Queen
  {{
    -20, -10, -10,  -5,  -5, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,   5,   5,   5,   0, -10,
     -5,   0,   5,   5,   5,   5,   0,  -5,
      0,   0,   5,   5,   5,   5,   0,  -5,
    -10,   5,   5,   5,   5,   5,   0, -10,
    -10,   0,   5,   0,   0,   0,   0, -10,
    -20, -10, -10,  -5,  -5, -10, -10, -20,
  }},
  // King (middlegame-oriented)
  {{
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
     20,  20,   0,   0,   0,   0,  20,  20,
     20,  30,  10,   0,   0,  10,  30,  20,
  }},
}};

};

Score eval(const Position& pos) {
  Score score = 0;

  score += material_score(pos);
  score += piece_square_score(pos);

  return pos.get_player() == Color::WHITE ? score : -score;
}

namespace {

Score material_score(const Position& pos) {
  Score score = 0;

  score += 100 * (pos.count_pieces(W_PAWN) - pos.count_pieces(B_PAWN));
  score += 300 * (pos.count_pieces(W_KNIGHT) - pos.count_pieces(B_KNIGHT));
  score += 320 * (pos.count_pieces(W_BISHOP) - pos.count_pieces(B_BISHOP));
  score += 500 * (pos.count_pieces(W_ROOK) - pos.count_pieces(B_ROOK));
  score += 900 * (pos.count_pieces(W_QUEEN) - pos.count_pieces(B_QUEEN));
  return score;
}

Score piece_square_score(const Position& pos) {
  Score score = 0;

  for (int type = 0; type < PIECE_TYPES; type++) {
    const PieceType piece = static_cast<PieceType>(type);
    const int pieceClass = piece_class_index(piece);
    if (pieceClass < 0) continue;

    Bitboard pieces = pos.get_bitboard_of(piece);
    while (pieces) {
      const Square sq = static_cast<Square>(__builtin_ctzll(pieces));
      pieces &= pieces - 1;

      const Square tableSq = is_white_piece(piece) ? sq : mirror_square(sq);
      const Score pst = kPieceSquareTables[pieceClass][tableSq];
      score += is_white_piece(piece) ? pst : -pst;
    }
  }

  return score;
}

}

