#ifndef POSITION_H
#define POSITION_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>

#include "bitboard.h"
#include "utils.h"

struct State {
  bool white = 1;
  bool castling_rights = 1;
  int rule50 = 0; // "halfmoves" 
  int fullMoves = 1;
  uint64_t enPassant;
};

// OBS! EMPTY not counted
constexpr uint8_t pieceTypes = 12;

enum PieceType { 
  B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
  W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
  EMPTY
};

class Position {
  public:
    Position() = default;
    void set(const std::string& FEN);
    void move();
    void undo_move();

    std::string fen() const;
    void print() const;

    inline int count_pieces(PieceType type) const;
    inline Bitboard get_bitboard(PieceType type) const;
  private:
    void set_bitboard(PieceType type);
    Bitboard pieces[pieceTypes];
    PieceType board[8][8];
};

inline Bitboard Position::get_bitboard(PieceType type) const {
  assert(type != PieceType::EMPTY);
  return pieces[type];
}

inline int Position::count_pieces(PieceType type) const {
  return __builtin_popcountll(pieces[type]);
}

#endif
