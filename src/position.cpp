#include <cassert>
#include "position.h"

const std::string charToPiece = "pnbrqkPNBRQK";

void Position::do_move(Move move, bool updateState) {
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

  // remove the captured piece from its bitboard
  PieceType dest_square = board[r][c];
  if (dest_square != PieceType::EMPTY) {
    Bitboards::clear_bit(piece_bitboard[dest_square], dest_idx);
  }
  board[r][c] = pc;

  if (updateState) {
    // change player
    state.white = !state.white;

    // update 50-move rule counter
    if (pc == W_PAWN || pc == B_PAWN || dest_square != PieceType::EMPTY) {
      state.rule50 = 0;
    } else {
      state.rule50++;
    }

    // set each bit in castling rights if the rook or king has moved
    if (pc == W_KING) {
      state.castling_rights &= 0b0011;
    } else if (pc == B_KING) {
      state.castling_rights &= 0b1100;
    } else if (pc == W_ROOK) {
      if (idx == 0) state.castling_rights &= 0b1110;
      else if (idx == 7) state.castling_rights &= 0b1101;
    } else if (pc == B_ROOK) {
      if (idx == 56) state.castling_rights &= 0b1011;
      else if (idx == 63) state.castling_rights &= 0b0111;
    }
  }
}

void Position::undo_move(Move move) {
}

std::string Position::fen() const {
  std::string fen = "";
  for (int i = 7; i >= 0; i--) {
    int empty = 0;
    for (int j = 0; j < 8; j++) {
      if (board[i][j] == PieceType::EMPTY) {
        empty++;
      } else {
        if (empty > 0) {
          fen += std::to_string(empty);
          empty = 0;
        }
        fen += charToPiece[board[i][j]];
      }
    }
    if (empty > 0) {
      fen += std::to_string(empty);
    }
    if (i > 0) fen += '/';
  }
  fen += " ";

  // player to move
  state.white ? fen += "w " : fen += "b ";

  // castling rights
  state.castling_rights & 0b0001 ? fen += 'K' : "";
  state.castling_rights & 0b0010 ? fen += 'Q' : "";
  state.castling_rights & 0b0100 ? fen += 'k' : "";
  state.castling_rights & 0b1000 ? fen += 'q' : "";

  // en passant square
  if (state.enPassant) {
    int ep_square = __builtin_ctzll(state.enPassant);
    char file = 'a' + (ep_square % 8);
    char rank = '1' + (ep_square / 8);
    fen += " ";
    fen += file;
    fen += rank;
  } else {
    fen += " -";
  }

  // 50-move rule counter
  fen += " " + std::to_string(state.rule50);

  // fullmove number
  fen += " " + std::to_string(state.fullMoves);

  return fen;
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
  printf("-----------------\n");
  for (int i = 7; i >= 0; i--) {
    printf("|");
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

Color Position::get_player() const {
  return state.white ? Color::WHITE : Color::BLACK;
}
