#include "movegen.h"

using namespace Bitboards;

MoveGen::MoveGen() {

}

std::vector<Move> MoveGen::generate_moves(PieceType piece, Square sq, const Position& pos) {
  Bitboard allWhite = pos.all_pieces(WHITE);
  Bitboard allBlack = pos.all_pieces(BLACK);
  std::vector<Move> moves;
  
  Bitboard legalSquares = attackMasks[piece][sq];

  // can't eat own pieces
  Bitboard occupied = is_white(piece) ? allWhite : allBlack;
  legalSquares &= ~occupied;

  // Bitboard to Moves
  for (Square square = 0; square < 64; square++) {
    if (legalSquares & (1ULL << square)) {
      Move move = {sq, square};
      moves.push_back(move);
    }
  }

  return moves;
}
