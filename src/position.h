#ifndef POSITION_H
#define POSITION_H

#include <algorithm>
#include <cassert>
#include <cmath>
#include <string>

#include "bitboard.h"
#include "types.h"
#include "utils.h"

struct State {
  bool white = 1;
  bool in_check = 0;
  unsigned int castling_rights : 4 = 0b1111;
  // 50 "halfmoves" 
  int rule50 = 0; 
  int fullMoves = 1;
  // bitboard with single 1 where en passant is possible
  Bitboard enPassant = 0; 
};

class Position {
public:
  Position() = default;

  void set(const std::string& FEN);
  void do_move(Move move, bool updateState = true);
  void undo_move(const Move move);

  // return position of all pieces of given color
  Bitboard all_pieces(Color color) const;
  Color get_player() const;
  std::string fen() const;
  void print_board() const;
  void print_state() const;

  inline void set_check() { state.in_check = 1; }

  inline bool is_check() const { return state.in_check; }

  inline PieceType piece_on(Square sq) const {
    return ((sq >= 0 && sq < 64) ? board[sq / 8][sq % 8] : PieceType::EMPTY);
  }

  inline int count_pieces(PieceType type) const { 
    return __builtin_popcountll(piece_bitboard[type]); 
  }

  inline Bitboard all_pieces() const {
    return all_pieces(WHITE) | all_pieces(BLACK);
  }

  inline Bitboard get_bitboard_of(PieceType type) const {
    return type == PieceType::EMPTY ? 0 : piece_bitboard[type];
  }

  inline unsigned int get_castling_rights() const {
    return state.castling_rights;
  }

private:
  void move_piece(Move& move);
  void update_state(const Move& move);

  State state;
  Bitboard piece_bitboard[pieceTypes];
  PieceType board[8][8];
  void set_bitboard(PieceType type);
};

#endif
