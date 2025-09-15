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
  unsigned int castling_rights : 4 = 0;
  // 50 "halfmoves" 
  int rule50 = 0; 
  int fullMoves = 1;
  // bitboard with single 1 where en passant is possible
  Bitboard enPassant; 
};

class Position {
  public:
    Position() = default;

    void set(const std::string& FEN);
    void do_move(Move move, bool updateState = true);
    void undo_move(Move move);
    Color get_player() const;

    std::string fen() const;
    void print() const;

    // return position of all pieces of given color
    Bitboard all_pieces(Color color) const;

    inline int count_pieces(PieceType type) const;
    inline Bitboard get_bitboard_of(PieceType type) const;

  private:
    State state;
    void set_bitboard(PieceType type);
    Bitboard piece_bitboard[pieceTypes];
    PieceType board[8][8];
};

inline Bitboard Position::get_bitboard_of(PieceType type) const {
  assert(type != PieceType::EMPTY);
  return piece_bitboard[type];
}

inline int Position::count_pieces(PieceType type) const {
  return __builtin_popcountll(piece_bitboard[type]);
}

#endif
