#ifndef POSITION_H
#define POSITION_H

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

#include "bitboard.h"
#include "types.h"
#include "utils.h"

struct State {
  bool white = 1;
  bool castling_rights_white = 0;
  bool castling_rights_black = 0;
  int rule50 = 0; // "halfmoves" 
  int fullMoves = 1;
  uint64_t enPassant;
};

class Position {
  public:
    Position() = default;

    void set(const std::string& FEN);
    void do_move(Move move);
    void undo_move(Move move);

    std::string fen() const;
    void print() const;

    // return position of all pieces of given color
    Bitboard all_pieces(Color color) const;

    inline int count_pieces(PieceType type) const;
    inline Bitboard get_bitboard(PieceType type) const;

  private:
    void set_bitboard(PieceType type);
    Bitboard piece_bitboard[pieceTypes];
    PieceType board[8][8];
};

inline Bitboard Position::get_bitboard(PieceType type) const {
  assert(type != PieceType::EMPTY);
  return piece_bitboard[type];
}

inline int Position::count_pieces(PieceType type) const {
  return __builtin_popcountll(piece_bitboard[type]);
}

#endif
