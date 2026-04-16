#include <cassert>
#include "bitboard.h"
#include "position.h"
#include "movegen.h"
#include "utils.h"

using namespace Bitboards;

bool Position::zobrist_initialized = false;
Key Position::zobrist_piece[PIECE_TYPES][64];
Key Position::zobrist_castling[1 << 4];
Key Position::zobrist_ep[8];
Key Position::zobrist_side;

void Position::init_zobrist() {
  if (zobrist_initialized) return;

  uint64_t seed = 0x9e3779b97f4a7c15ULL;

  for (int piece = 0; piece < PIECE_TYPES; piece++) {
    for (int sq = 0; sq < 64; sq++) {
      zobrist_piece[piece][sq] = splitmix64(seed);
    }
  }

  for (int cr = 0; cr < (1 << 4); cr++) {
    zobrist_castling[cr] = splitmix64(seed);
  }

  for (int file = 0; file < 8; file++) {
    zobrist_ep[file] = splitmix64(seed);
  }

  zobrist_side = splitmix64(seed);
  zobrist_initialized = true;
}

int Position::hashable_ep_file() const {
  if (!state.enPassant) return -1;

  // Pseudo-legal en passant hashing (not strict legality):
  // include the EP file only if the side to move has a pawn that could
  // capture the EP target square by pawn attack geometry.
  const int ep = __builtin_ctzll(state.enPassant);
  const int file = ep % 8;

  if (state.white) {
    if (file > 0) {
      const int from = ep - 9;
      if (from >= 0 && board[from] == W_PAWN) return file;
    }
    if (file < 7) {
      const int from = ep - 7;
      if (from >= 0 && board[from] == W_PAWN) return file;
    }
  } else {
    if (file > 0) {
      const int from = ep + 7;
      if (from < 64 && board[from] == B_PAWN) return file;
    }
    if (file < 7) {
      const int from = ep + 9;
      if (from < 64 && board[from] == B_PAWN) return file;
    }
  }

  return -1;
}

Key Position::compute_hash() const {
  Key h = 0;

  for (Square sq = 0; sq < 64; sq++) {
    PieceType piece = board[sq];
    if (piece != EMPTY) {
      h ^= zobrist_piece[piece][sq];
    }
  }

  h ^= zobrist_castling[state.castling_rights & 0xF];

  const int ep_file = hashable_ep_file();
  if (ep_file >= 0) {
    h ^= zobrist_ep[ep_file];
  }

  if (!state.white) {
    h ^= zobrist_side;
  }

  return h;
}

Position::UndoInfo Position::do_move(Move move, bool updateState) {
  uint8_t from_idx = move.from;  // Already a square index (0-63)
  PieceType movedPiece = board[from_idx];

  Position::UndoInfo undo;
  undo.prev_state = state;
  undo.move = move;
  undo.moved_piece = movedPiece;
  undo.captured_square = move.to;
  undo.prev_key = key;
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
    const uint8_t prev_castling = state.castling_rights;
    const int prev_ep_file = hashable_ep_file();

    update_state(move, movedPiece);

    key ^= zobrist_side;
    key ^= zobrist_castling[prev_castling & 0xF];
    key ^= zobrist_castling[state.castling_rights & 0xF];

    if (prev_ep_file >= 0) key ^= zobrist_ep[prev_ep_file];
    const int next_ep_file = hashable_ep_file();
    if (next_ep_file >= 0) key ^= zobrist_ep[next_ep_file];
  }

  undo.move = move;
  return undo;
}

void Position::undo_move(const Position::UndoInfo& undo) {
  state = undo.prev_state;

  const Move& move = undo.move;
  Square from_idx = move.from;
  Square to_idx = move.to;

  PieceType moved_now = board[to_idx];

  piece_bitboard[moved_now] = clear_bit(piece_bitboard[moved_now], to_idx);
  board[to_idx] = EMPTY;

  PieceType restored_mover = undo.moved_piece;
  piece_bitboard[restored_mover] = set_bit(piece_bitboard[restored_mover], from_idx);
  board[from_idx] = restored_mover;

  if (move.flags & MoveFlags::KING_CASTLE) {
    Square rook_from = piece_is_white(restored_mover) ? 5 : 61;
    Square rook_to = piece_is_white(restored_mover) ? 7 : 63;
    PieceType rook = board[rook_from];

    piece_bitboard[rook] = clear_bit(piece_bitboard[rook], rook_from);
    piece_bitboard[rook] = set_bit(piece_bitboard[rook], rook_to);
    board[rook_from] = EMPTY;
    board[rook_to] = rook;
  } else if (move.flags & MoveFlags::QUEEN_CASTLE) {
    Square rook_from = piece_is_white(restored_mover) ? 3 : 59;
    Square rook_to = piece_is_white(restored_mover) ? 0 : 56;
    PieceType rook = board[rook_from];

    piece_bitboard[rook] = clear_bit(piece_bitboard[rook], rook_from);
    piece_bitboard[rook] = set_bit(piece_bitboard[rook], rook_to);
    board[rook_from] = EMPTY;
    board[rook_to] = rook;
  }

  if (move.captured_piece != EMPTY) {
    Square cap_idx = undo.captured_square;
    board[cap_idx] = move.captured_piece;
    piece_bitboard[move.captured_piece] = set_bit(piece_bitboard[move.captured_piece], cap_idx);
  }

  key = undo.prev_key;
}

void Position::move_piece(Move& move) {
  Square from_idx = move.from;  // Already a square index (0-63)
  PieceType pc = board[from_idx];

  key ^= zobrist_piece[pc][from_idx];

  // Clear piece from source square bitboard
  piece_bitboard[pc] = clear_bit(piece_bitboard[pc], from_idx);

  Square to_idx = move.to;  // Already a square index (0-63)

  // Remove captured piece from its bitboard (if any), including en-passant captures.
  move.captured_piece = PieceType::EMPTY;
  if ((move.flags & MoveFlags::EN_PASSANT) && (pc == W_PAWN || pc == B_PAWN)) {
    Square captured_idx = (pc == W_PAWN) ? static_cast<Square>(to_idx - 8)
                                         : static_cast<Square>(to_idx + 8);
    PieceType expected_captured_pawn = (pc == W_PAWN) ? B_PAWN : W_PAWN;
    PieceType captured_piece = board[captured_idx];

    // En passant always captures an enemy pawn.
    move.captured_piece = expected_captured_pawn;
    if (captured_piece != PieceType::EMPTY) {
      key ^= zobrist_piece[captured_piece][captured_idx];
      piece_bitboard[captured_piece] = clear_bit(piece_bitboard[captured_piece], captured_idx);
      board[captured_idx] = PieceType::EMPTY;
    }
  } else {
    PieceType captured_piece = board[to_idx];
    move.captured_piece = captured_piece;
    if (captured_piece != PieceType::EMPTY) {
      key ^= zobrist_piece[captured_piece][to_idx];
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
  board[from_idx] = PieceType::EMPTY;
  board[to_idx] = placedPiece;

  key ^= zobrist_piece[placedPiece][to_idx];
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
      const Square sq = static_cast<Square>(i * 8 + j);
      if (board[sq] == PieceType::EMPTY) {
        empty++;
      } else {
        if (empty > 0) {
          fen += std::to_string(empty);
          empty = 0;
        }
        fen += charToPiece[board[sq]];
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
  init_zobrist();

  // Reset board and state before parsing.
  for (int sq = 0; sq < 64; sq++) {
    board[sq] = EMPTY;
  }
  for (int i = 0; i < PIECE_TYPES; i++) {
    piece_bitboard[i] = 0;
  }
  state = State{};

  // Support both full FEN and board-only FEN.
  std::string boardPart;
  std::string sidePart;
  std::string castlingPart;
  std::string epPart;
  std::string rule50Part;
  std::string fullMovesPart;

  size_t p1 = FEN.find(' ');
  if (p1 == std::string::npos) {
    boardPart = FEN;
  } else {
    boardPart = FEN.substr(0, p1);

    size_t p2 = FEN.find(' ', p1 + 1);
    if (p2 == std::string::npos) p2 = FEN.size();
    sidePart = FEN.substr(p1 + 1, p2 - (p1 + 1));

    if (p2 < FEN.size()) {
      size_t p3 = FEN.find(' ', p2 + 1);
      if (p3 == std::string::npos) p3 = FEN.size();
      castlingPart = FEN.substr(p2 + 1, p3 - (p2 + 1));

      if (p3 < FEN.size()) {
        size_t p4 = FEN.find(' ', p3 + 1);
        if (p4 == std::string::npos) p4 = FEN.size();
        epPart = FEN.substr(p3 + 1, p4 - (p3 + 1));

        if (p4 < FEN.size()) {
          size_t p5 = FEN.find(' ', p4 + 1);
          if (p5 == std::string::npos) p5 = FEN.size();
          rule50Part = FEN.substr(p4 + 1, p5 - (p4 + 1));
          if (p5 < FEN.size()) {
            fullMovesPart = FEN.substr(p5 + 1);
          }
        }
      }
    }
  }

  int rank = 7;
  int file = 0;
  for (char c : boardPart) {
    if (isalpha(c)) {
      auto it = std::find(charToPiece.begin(), charToPiece.end(), c);
      assert(it != charToPiece.end());
      PieceType pc = (PieceType) std::distance(charToPiece.begin(), it);
      assert(rank >= 0 && rank < 8 && file >= 0 && file < 8);
      board[rank * 8 + file++] = pc;
    } else if (isdigit(c)) {
      int cnt = c - '0';
      for (int i = 0; i < cnt; i++) {
        assert(rank >= 0 && rank < 8 && file >= 0 && file < 8);
        board[rank * 8 + file++] = PieceType::EMPTY;
      }
    } else if (c == '/') {
      assert(file == 8);
      file = 0;
      rank--;
    }
  }

  // Parse side to move.
  if (!sidePart.empty()) {
    state.white = (sidePart == "w");
  }

  // Parse castling rights.
  if (!castlingPart.empty()) {
    state.castling_rights = 0;
    if (castlingPart.find('K') != std::string::npos) state.castling_rights |= WK;
    if (castlingPart.find('Q') != std::string::npos) state.castling_rights |= WQ;
    if (castlingPart.find('k') != std::string::npos) state.castling_rights |= BK;
    if (castlingPart.find('q') != std::string::npos) state.castling_rights |= BQ;
  }

  // Parse en passant target square.
  if (!epPart.empty() && epPart != "-" && epPart.size() == 2) {
    char f = epPart[0];
    char r = epPart[1];
    if (f >= 'a' && f <= 'h' && r >= '1' && r <= '8') {
      Square sq = (Square) ((f - 'a') + (r - '1') * 8);
      state.enPassant = (1ULL << sq);
    }
  }

  if (!rule50Part.empty()) state.rule50 = std::stoi(rule50Part);
  if (!fullMovesPart.empty()) state.fullMoves = std::stoi(fullMovesPart);

  // Initialize bitboards for pieces.
  for (int type = 0; type < PIECE_TYPES; type++) {
    set_bitboard((PieceType) type);
  }

  key = compute_hash();
}

void Position::set_bitboard(PieceType type) {
  assert(type != PieceType::EMPTY);
  Bitboard bitboard = 0;

  for (Square sq = 0; sq < 64; sq++) {
    if (board[sq] == type) {
      bitboard |= (1ULL << sq);
    }
  }
  piece_bitboard[type] = bitboard;
}

void Position::print_board() const {
  printf("---------------------------------\n");
  for (int i = 7; i >= 0; i--) {
    printf("|");
    for (int j = 0; j < 8; j++) {
      PieceType piece = board[i * 8 + j];
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
