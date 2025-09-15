#include "movegen.h"

using namespace Bitboards;

MoveGen::MoveGen() {

}

Bitboard pseudo_legal_moves(PieceType piece, Square sq, const Position& pos) {
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
  Bitboard occupied = piece_is_white(piece) ? allWhite : allBlack;
  legalSquares &= ~occupied;

  return legalSquares;
}

std::vector<Move> bitboard_to_moves(Bitboard board, Square from) {
  std::vector<Move> moves;
  for (Square to = 0; to < 64; to++) {
    if (board & (1ULL << to)) {
      Move move = {from, to};
      moves.push_back(move);
    }
  }
  return moves;
}

std::vector<Move> MoveGen::generate_moves(PieceType piece, Square sq, const Position& pos) const {
  // legal squares for the piece on sq
  Bitboard legalSquares = pseudo_legal_moves(piece, sq, pos);

  // Bitboard to Moves
  std::vector<Move> moves = bitboard_to_moves(legalSquares, sq);

  return moves;
}