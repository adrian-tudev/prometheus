#include "movegen.h"

using namespace Bitboards;

MoveGen::MoveGen() {

}

std::vector<Move> MoveGen::generate_moves(PieceType piece, Square sq, const Position& pos) {
  Bitboard allWhite = pos.all_pieces(WHITE);
  Bitboard allBlack = pos.all_pieces(BLACK);
  Bitboard allPieces = allWhite | allBlack;
  
  Bitboard legalSquares = movementMasks[piece][sq];

  if (is_sliding(piece)) {
    Bitboard blockers = allPieces & movementMasks[piece][sq];
    if (piece == W_ROOK || piece == B_ROOK)
      legalSquares = rookAttack[{sq, blockers}];
    else if (piece == W_BISHOP || piece == B_BISHOP)
      legalSquares = bishopAttack[{sq, blockers}];
    else if (piece == W_QUEEN || piece == B_QUEEN) {
      blockers = allPieces & movementMasks[W_BISHOP][sq];
      legalSquares = bishopAttack[{sq, blockers}];

      blockers = allPieces & movementMasks[W_ROOK][sq];
      legalSquares |= rookAttack[{sq, blockers}];
    }
  }
  // can't eat own pieces
  Bitboard occupied = is_white(piece) ? allWhite : allBlack;
  legalSquares &= ~occupied;

  // Bitboard to Moves
  std::vector<Move> moves;
  for (Square square = 0; square < 64; square++) {
    if (legalSquares & (1ULL << square)) {
      Move move = {sq, square};
      moves.push_back(move);
    }
  }
  print(legalSquares);

  return moves;
}
