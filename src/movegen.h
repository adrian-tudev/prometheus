#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <cstdint>
#include <vector>

#include "bitboard.h"
#include "position.h"
#include "types.h"
#include "utils.h"

using std::vector;

class MoveGen {
public:
  MoveGen() = default;
  ~MoveGen() = default;

  // all moves for current player
  vector<Move> generate_moves(const Position& pos);

  // all moves for a specific piece
  vector<Move> generate_moves_at(Square sq, const Position& pos);

private:
  Bitboard pseudo_legal_moves(PieceType piece, Square sq, const Position& pos);
  vector<Move> castling(const Position& pos, Bitboard enemyAttacks);
  vector<Move> bitboard_to_moves(Bitboard board, Square from);
  Bitboard pawn_attacks(Color color, Square sq);
  Bitboard attack_mask(const Position& pos);
};

#endif
