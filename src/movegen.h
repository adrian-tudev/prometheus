#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <cstdint>
#include <vector>

#include "bitboard.h"
#include "position.h"
#include "types.h"
#include "utils.h"

class MoveGen {
public:
  MoveGen() = default;
  ~MoveGen() = default;

  // all moves for current player
  std::vector<Move> generate_moves(const Position& pos);

  // all moves for a specific piece
  std::vector<Move> generate_moves_at(Square sq, const Position& pos);

private:
  Bitboard pseudo_legal_moves(PieceType piece, Square sq, const Position& pos);
  Bitboard castling(const Position& pos);
  std::vector<Move> bitboard_to_moves(Bitboard board, Square from);
  Bitboard pawn_attacks(Color color, Square sq);
  Bitboard attack_mask(const Position& pos);
};

#endif
