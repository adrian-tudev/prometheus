#include <cassert>
#include "bitboard.h"
#include "position.h"
#include "movegen.h"
#include "utils.h"

using namespace Bitboards;

namespace {

struct SquareCoord {
  Square rank;
  Square file;
};

inline SquareCoord to_coord(Square idx) {
  return SquareCoord{static_cast<Square>(idx / 8), static_cast<Square>(idx % 8)};
}

} // namespace

// TODO: update Move flags accordingly
Position::UndoInfo Position::do_move(Move move, bool updateState) {
  uint8_t from_idx = move.from;  // Already a square index (0-63)
  SquareCoord from = to_coord(from_idx);
  PieceType movedPiece = board[from.rank][from.file];

  Position::UndoInfo undo;
  undo.prev_state = state;
  undo.move = move;
  undo.moved_piece = movedPiece;
  undo.captured_square = move.to;
  if ((move.flags & MoveFlags::EN_PASSANT) && (movedPiece == W_PAWN || movedPiece == B_PAWN)) {
    undo.captured_square = (movedPiece == W_PAWN) ? static_cast<Square>(move.to - 8)
                                                  : static_cast<Square>(move.to + 8);
  }

  move_piece(move);

  if (move.flags & MoveFlags::KING_CASTLE) {
    Move rookMove;
    rookMove.from = piece_is_white(movedPiece) ? 7 : 63;
    rookMove.to = piece_is_white(movedPiece) ? 5 : 61;
    move_piece(rookMove);
  }
  if (move.flags & MoveFlags::QUEEN_CASTLE) {
    Move rookMove;
    rookMove.from = piece_is_white(movedPiece) ? 0 : 56;
    rookMove.to = piece_is_white(movedPiece) ? 3 : 59;
    move_piece(rookMove);
  }

  if (updateState) {
    update_state(move, movedPiece);
  }

  undo.move = move;
  return undo;
}

void Position::undo_move(const Position::UndoInfo& undo) {
  state = undo.prev_state;

  const Move& move = undo.move;
  Square from_idx = move.from;
  Square to_idx = move.to;

  SquareCoord from = to_coord(from_idx);
  SquareCoord to = to_coord(to_idx);

  PieceType moved_now = board[to.rank][to.file];

  piece_bitboard[moved_now] = clear_bit(piece_bitboard[moved_now], to_idx);
  board[to.rank][to.file] = EMPTY;

  PieceType restored_mover = undo.moved_piece;
  piece_bitboard[restored_mover] = set_bit(piece_bitboard[restored_mover], from_idx);
  board[from.rank][from.file] = restored_mover;

  if (move.flags & MoveFlags::KING_CASTLE) {
    Square rook_from = piece_is_white(restored_mover) ? 5 : 61;
    Square rook_to = piece_is_white(restored_mover) ? 7 : 63;
    SquareCoord rookFrom = to_coord(rook_from);
    SquareCoord rookTo = to_coord(rook_to);
    PieceType rook = board[rookFrom.rank][rookFrom.file];

    piece_bitboard[rook] = clear_bit(piece_bitboard[rook], rook_from);
    piece_bitboard[rook] = set_bit(piece_bitboard[rook], rook_to);
    board[rookFrom.rank][rookFrom.file] = EMPTY;
    board[rookTo.rank][rookTo.file] = rook;
  } else if (move.flags & MoveFlags::QUEEN_CASTLE) {
    Square rook_from = piece_is_white(restored_mover) ? 3 : 59;
    Square rook_to = piece_is_white(restored_mover) ? 0 : 56;
    SquareCoord rookFrom = to_coord(rook_from);
    SquareCoord rookTo = to_coord(rook_to);
    PieceType rook = board[rookFrom.rank][rookFrom.file];

    piece_bitboard[rook] = clear_bit(piece_bitboard[rook], rook_from);
    piece_bitboard[rook] = set_bit(piece_bitboard[rook], rook_to);
    board[rookFrom.rank][rookFrom.file] = EMPTY;
    board[rookTo.rank][rookTo.file] = rook;
  }

  if (move.captured_piece != EMPTY) {
    Square cap_idx = undo.captured_square;
    SquareCoord cap = to_coord(cap_idx);
    board[cap.rank][cap.file] = move.captured_piece;
    piece_bitboard[move.captured_piece] = set_bit(piece_bitboard[move.captured_piece], cap_idx);
  }
}

void Position::move_piece(Move& move) {
  Square from_idx = move.from;  // Already a square index (0-63)
  SquareCoord from = to_coord(from_idx);
  PieceType pc = board[from.rank][from.file];

  // Clear piece from source square bitboard
  piece_bitboard[pc] = clear_bit(piece_bitboard[pc], from_idx);

  Square to_idx = move.to;  // Already a square index (0-63)
  SquareCoord to = to_coord(to_idx);

  // Remove captured piece from its bitboard (if any), including en-passant captures.
  move.captured_piece = PieceType::EMPTY;
  if ((move.flags & MoveFlags::EN_PASSANT) && (pc == W_PAWN || pc == B_PAWN)) {
    Square captured_idx = (pc == W_PAWN) ? static_cast<Square>(to_idx - 8)
                                         : static_cast<Square>(to_idx + 8);
    SquareCoord captured = to_coord(captured_idx);
    PieceType expected_captured_pawn = (pc == W_PAWN) ? B_PAWN : W_PAWN;
    PieceType captured_piece = board[captured.rank][captured.file];

    // En passant always captures an enemy pawn.
    move.captured_piece = expected_captured_pawn;
    if (captured_piece != PieceType::EMPTY) {
      piece_bitboard[captured_piece] = clear_bit(piece_bitboard[captured_piece], captured_idx);
      board[captured.rank][captured.file] = PieceType::EMPTY;
    }
  } else {
    PieceType captured_piece = board[to.rank][to.file];
    move.captured_piece = captured_piece;
    if (captured_piece != PieceType::EMPTY) {
      piece_bitboard[captured_piece] = clear_bit(piece_bitboard[captured_piece], to_idx);
    }
  }

  PieceType placedPiece = pc;
  if (move.flags & PROMOTION) {
    placedPiece = move.promotion;
  }

  // Set piece at destination square bitboard
  piece_bitboard[placedPiece] = set_bit(piece_bitboard[placedPiece], to_idx);

  // Update board array
  board[from.rank][from.file] = PieceType::EMPTY;
  board[to.rank][to.file] = placedPiece;
}

void Position::update_state(const Move& move, PieceType movedPiece) {
  Square from_idx = move.from;  // Already a square index (0-63)
  Square to_idx = move.to;
  PieceType pc = movedPiece;

  // En passant rights are valid for one ply and only after a double pawn push.
  state.enPassant = 0;
  if ((move.flags & MoveFlags::DOUBLE_PAWN_PUSH) && (pc == W_PAWN || pc == B_PAWN)) {
    Square ep_square = (pc == W_PAWN) ? static_cast<Square>(from_idx + 8)
                                      : static_cast<Square>(from_idx - 8);
    state.enPassant = 1ULL << ep_square;
  }

  state.fullMoves += !state.white;

  // change player
  state.white = !state.white;

  // update 50-move rule counter
  if (pc == W_PAWN || pc == B_PAWN || move.captured_piece != PieceType::EMPTY) {
    state.rule50 = 0;
  } else {
    state.rule50++;
  }

  // update castling rights if king or rook moves
  if (pc == W_KING) {
    state.castling_rights &= ~(CastlingRights::WK | CastlingRights::WQ);
  } else if (pc == B_KING) {
    state.castling_rights &= ~(CastlingRights::BK | CastlingRights::BQ);
  } else if (pc == W_ROOK) {
    if (from_idx == 0) state.castling_rights &= ~CastlingRights::WQ;  // a1 rook moved
    else if (from_idx == 7) state.castling_rights &= ~CastlingRights::WK;  // h1 rook moved
  } else if (pc == B_ROOK) {
    if (from_idx == 56) state.castling_rights &= ~CastlingRights::BQ;  // a8 rook moved
    else if (from_idx == 63) state.castling_rights &= ~CastlingRights::BK;  // h8 rook moved
  }

  // captured rook on original square also removes castling rights
  if (move.captured_piece == W_ROOK) {
    if (to_idx == 0) state.castling_rights &= ~CastlingRights::WQ;
    else if (to_idx == 7) state.castling_rights &= ~CastlingRights::WK;
  } else if (move.captured_piece == B_ROOK) {
    if (to_idx == 56) state.castling_rights &= ~CastlingRights::BQ;
    else if (to_idx == 63) state.castling_rights &= ~CastlingRights::BK;
  }

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

void Position::print_board() const {
  printf("---------------------------------\n");
  for (int i = 7; i >= 0; i--) {
    printf("|");
    for (int j = 0; j < 8; j++) {
      PieceType piece = board[i][j];
      if (piece == PieceType::EMPTY) {
        std::cout << "   |";
      } else if (piece >= 0 && piece < charToPiece.size()) {
        std::cout << " " << charToPiece[piece] << " |";
      } else {
        std::cout << " ? |";  // Invalid piece type
      }
    }
    std::cout << "\n---------------------------------\n";
  }
  std::cout << std::endl;
}

Color Position::get_player() const {
  return state.white ? Color::WHITE : Color::BLACK;
}

bool Position::is_check() const {
  return is_in_check(get_player());
}

bool Position::is_in_check(Color player) const {
  Color enemy = player == WHITE ? BLACK : WHITE;
  Bitboard king = player == WHITE ? get_bitboard_of(W_KING) : get_bitboard_of(B_KING);

  Bitboard enemyAttacks = MoveGen::attack_mask(*this, enemy);
  return (enemyAttacks & king) != 0;
}

bool Position::is_check_mate() const {
  std::vector<Move> moves = MoveGen::generate_moves(*this);
  return moves.empty();
}
