#ifndef POSITION_H
#define POSITION_H

#include <cassert>
#include <cstdint>
#include <string>

#include "types.h"

struct State {
  bool white = 1;
  uint8_t castling_rights : 4 = CastlingRights::ALL;
  // 50 "halfmoves" 
  int rule50 = 0; 
  int fullMoves = 1;
  // bitboard with single 1 where en passant is possible
  Bitboard enPassant = 0; 
};

class Position {
public:
  struct UndoInfo {
    State prev_state;
    Move move;
    PieceType moved_piece = EMPTY;
    Square captured_square = 0;
    Key prev_key = 0;
  };

  Position() = default;

  void set(const std::string& FEN);
  UndoInfo do_move(Move move, bool updateState = true);
  void undo_move(const UndoInfo& undo);

  // return position of all pieces of given color
  Bitboard all_pieces(Color color) const;
  Color get_player() const;
  std::string fen() const;
  void print_board() const;

  // returns true if the current player is in check
  bool is_check() const;
  bool is_in_check(Color player) const;

  // returns true if the current player is checkmated
  bool is_check_mate() const;

  inline PieceType piece_on(Square sq) const {
    return (sq < 64) ? board[sq] : PieceType::EMPTY;
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

  inline const State& get_state() const {
    return state;
  }

  inline Key hash() const {
    return key;
  }

private:
  void move_piece(Move& move);
  void update_state(const Move& move, PieceType movedPiece);
  void set_bitboard(PieceType type);
  void init_zobrist();
  Key compute_hash() const;
  int ep_file_from_mask(Bitboard epMask) const;

  static bool zobrist_initialized;
  static Key zobrist_piece[PIECE_TYPES][64];
  static Key zobrist_castling[1 << 4];
  static Key zobrist_ep[8];
  static Key zobrist_side;

  State state;
  Bitboard piece_bitboard[PIECE_TYPES];
  PieceType board[64];
  Key key = 0;
};

#endif
