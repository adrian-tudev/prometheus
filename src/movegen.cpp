#include "movegen.h"

using namespace Bitboards;

MoveGen::MoveGen() {

}

std::vector<Move> MoveGen::generate_moves(PieceType piece, Square square) {
  std::vector<Move> moves;
  Bitboard attackSquares = pieceMovement[piece][square];

  return moves;
}
