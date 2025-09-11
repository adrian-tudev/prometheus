#include <cassert>
#include "position.h"

const std::string charToPiece = "pnbrqkPNBRQK";

void Position::do_move(Move move) {
  uint8_t idx = std::log2(move.target);
  uint8_t r = idx / 8;
  uint8_t c = idx % 8;
  PieceType pc = board[r][c];

  Bitboards::clear_bit(piece_bitboard[pc], idx);

  uint8_t dest_idx = std::log2(move.dest);
  Bitboards::set_bit(piece_bitboard[pc], dest_idx);
  board[r][c] = PieceType::EMPTY;
  r = dest_idx / 8;
  c = dest_idx % 8;
  board[r][c] = pc;
}

void Position::undo_move(Move move) {
}

Bitboard Position::all_pieces(Color color) const {
  Bitboard all = 0;
  int start = color == WHITE ? W_PAWN : B_PAWN;
  int end = start + 6;
  for (int i = start; i < end; i++) {
    all |= piece_bitboard[i];
  }
  return all;
}

void Position::set(const std::string& FEN) {
  int rank = 7;
  int file = 0;
  for (char c : FEN) {
    if (isalpha(c)) {
      // piece
      auto it = std::find(charToPiece.begin(), charToPiece.end(), c);
      assert(it != charToPiece.end());
      PieceType pc = (PieceType) std::distance(charToPiece.begin(), it);
      board[rank][file] = pc;
      file++;
    } else if (isdigit(c)) {
      // empty squares
      for (int i = 0; i < c - '0'; i++) {
        board[rank][file++] = PieceType::EMPTY;
      }
    } else {
      // new rank '/'
      file = 0;
      rank--;
    }
  }
  
  // Initialize bitboards for pieces
  for (int type = 0; type < pieceTypes; type++) {
    set_bitboard((PieceType) type);
  }
}

void Position::set_bitboard(PieceType type) {
  assert(type != PieceType::EMPTY);
  Bitboard bitboard = 0;

  uint8_t cnt = 0;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (board[i][j] == type) {
        bitboard |= (1ull << cnt);
      }
      cnt++;
    }
  }
  piece_bitboard[type] = bitboard;
}

void Position::print() const {
  std::cout << "-----------------\n";
  for (int i = 7; i >= 0; i--) {
    std::cout << "|";
    for (int j = 0; j < 8; j++) {
      if (board[i][j] == PieceType::EMPTY) {
        std::cout << ' ' << '|';
        continue;
      }
      std::cout << charToPiece[board[i][j]] << '|';
    }
    std::cout << "\n-----------------\n";
  }
  std::cout << std::endl;
}
