#include "movegen.h"

using namespace Bitboards;

MoveGen::MoveGen() {

}

std::vector<Move> MoveGen::generate_moves(PieceType piece, Square square, Bitboard allWhite, Bitboard allBlack) {
  std::vector<Move> moves;
  
  // pseudo-legal moves
  Bitboard legalSquares = pieceMovement[piece][square];

  // can't eat own pieces
  Bitboard occupied = is_white(piece) ? allWhite : allBlack;
  legalSquares &= ~occupied;

  // Bitboard to Moves
  for (int i = 0; i < 64; i++) {
    if (legalSquares & (1ULL << i)) {
      Move move = {(Square)(1ULL << square), (Square)(1ULL << i)};
      moves.push_back(move);
    }
  }

  return moves;
}
