#include <cassert>
#include "position.h"

void Position::do_move(Move move, bool updateState) {
  uint8_t from_idx = move.from;  // Already a square index (0-63)
  uint8_t from_r = from_idx / 8;
  uint8_t from_c = from_idx % 8;
  PieceType pc = board[from_r][from_c];

  // Clear piece from source square bitboard
  piece_bitboard[pc] = Bitboards::clear_bit(piece_bitboard[pc], from_idx);

  uint8_t to_idx = move.to;  // Already a square index (0-63)
  uint8_t to_r = to_idx / 8;
  uint8_t to_c = to_idx % 8;

  // Remove captured piece from its bitboard (if any)
  PieceType captured_piece = board[to_r][to_c];
  if (captured_piece != PieceType::EMPTY) {
    piece_bitboard[captured_piece] = Bitboards::clear_bit(piece_bitboard[captured_piece], to_idx);
  }

  // Set piece at destination square bitboard
  piece_bitboard[pc] = Bitboards::set_bit(piece_bitboard[pc], to_idx);

  // Update board array
  board[from_r][from_c] = PieceType::EMPTY;
  board[to_r][to_c] = pc;

  if (updateState) {
    // change player
    state.white = !state.white;

    // update 50-move rule counter
    if (pc == W_PAWN || pc == B_PAWN || captured_piece != PieceType::EMPTY) {
      state.rule50 = 0;
    } else {
      state.rule50++;
    }

    // update castling rights if king or rook moves
    if (pc == W_KING) {
      state.castling_rights &= 0b0011;  // Clear white castling rights
    } else if (pc == B_KING) {
      state.castling_rights &= 0b1100;  // Clear black castling rights
    } else if (pc == W_ROOK) {
      if (from_idx == 0) state.castling_rights &= 0b1110;  // a1 rook moved
      else if (from_idx == 7) state.castling_rights &= 0b1101;  // h1 rook moved
    } else if (pc == B_ROOK) {
      if (from_idx == 56) state.castling_rights &= 0b1011;  // a8 rook moved
      else if (from_idx == 63) state.castling_rights &= 0b0111;  // h8 rook moved
    }
  }
}

void Position::undo_move(Move move) {
  printf("undo move not implemented yet!\n");
  assert(false);
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
      PieceType piece = board[i][j];
      if (piece == PieceType::EMPTY) {
        std::cout << ' ' << '|';
      } else if (piece >= 0 && piece < charToPiece.size()) {
        std::cout << charToPiece[piece] << '|';
      } else {
        std::cout << '?' << '|';  // Invalid piece type
      }
    }
    std::cout << "\n-----------------\n";
  }
  std::cout << std::endl;
}

Color Position::get_player() const {
  return state.white ? Color::WHITE : Color::BLACK;
}
