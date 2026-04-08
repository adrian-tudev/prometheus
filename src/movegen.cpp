#include "movegen.h"

#include "bitboard.h"
#include "utils.h"

using namespace Bitboards;

namespace MoveGen {

Bitboard attack_mask(const Position& pos, Color player);

namespace {

Bitboard pseudo_legal_moves(PieceType piece, Square sq, const Position& pos);
vector<Move> castling(const Position& pos, Bitboard enemyAttacks);
vector<Move> bitboard_to_moves(Bitboard board, Square from);
Bitboard pawn_attacks(Color color, Square sq);

} // namespace

vector<Move> generate_moves(const Position& pos) {
  std::vector<Move> moves;
  Color player = pos.get_player();
  Bitboard pieces = pos.all_pieces(player);

  while (pieces) {
    Square sq = __builtin_ctzll(pieces);
    pieces &= pieces - 1; // clear lsb

    vector<Move> pieceMoves = generate_moves_at(sq, pos);
    moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
  }

  return moves;
}

vector<Move> generate_moves_at(Square sq, const Position& pos) {
  PieceType piece = pos.piece_on(sq);
  if (piece == PieceType::EMPTY) 
    return std::vector<Move>();

  Color player = get_color(piece);
  Color enemy = static_cast<Color>(!player);
  State state = pos.get_state();
  // legal squares for the piece on sq
  Bitboard legal_squares = pseudo_legal_moves(piece, sq, pos);

  // attacks for pawns (different from movement)
  if (piece == W_PAWN || piece == B_PAWN) {
    Bitboard attacks = pawn_attacks(get_color(piece), sq);
    Bitboard capturable = pos.all_pieces((Color)!get_color(piece));
    legal_squares |= capturable & attacks;

    // En passant is only available if this pawn attacks the en-passant target square.
    legal_squares |= state.enPassant & attacks;
  }

  // king stuff
  vector<Move> castling_moves;
  if (piece == W_KING || piece == B_KING) {
    Bitboard enemyAttack = attack_mask(pos, enemy);

    // prevent the king from walking into enemy attacks
    legal_squares &= ~enemyAttack;

    // add castling
    castling_moves = castling(pos, enemyAttack);
  }

  // can't eat own pieces
  // important: do this after checking for checks to disable the king from capturing a "defended" piece
  Bitboard occupied = pos.all_pieces(player);
  legal_squares &= ~occupied;

  // Bitboard to Moves
  vector<Move> moves = bitboard_to_moves(legal_squares, sq);

  // add flags for generated moves
  Bitboard enemyPieces = pos.all_pieces(enemy);
  for (Move& move : moves) {
    if (enemyPieces & (1ULL << move.to)) {
      move.flags = static_cast<MoveFlags>(move.flags | CAPTURE);
    }

    if ((piece == W_PAWN || piece == B_PAWN) &&
        (state.enPassant & (1ULL << move.to))) {
      move.flags = static_cast<MoveFlags>(move.flags | EN_PASSANT | CAPTURE);
    }

    if ((piece == W_PAWN && move.to == move.from + 16) ||
        (piece == B_PAWN && move.to + 16 == move.from)) {
      move.flags = static_cast<MoveFlags>(move.flags | DOUBLE_PAWN_PUSH);
    }

    Position next = pos;
    next.do_move(move, false);
    if (next.is_in_check(enemy)) {
      move.flags = static_cast<MoveFlags>(move.flags | CHECK);
    }

    // TODO: promotions
  }

  moves.insert(moves.end(), castling_moves.begin(), castling_moves.end());

  // remove moves that put the player in check
  std::vector<Move> in_check_moves;
  for (const Move move : moves) {
    Position next = pos;
    next.do_move(move, false);
    if (!next.is_check()) {
      in_check_moves.push_back(move);
    }
  }
  moves = in_check_moves;

  return moves;
}

namespace {

Bitboard pseudo_legal_moves(PieceType piece, Square sq, const Position& pos) {
  Bitboard allWhite = pos.all_pieces(WHITE);
  Bitboard allBlack = pos.all_pieces(BLACK);
  Bitboard allPieces = allWhite | allBlack;
  
  Bitboard legal_squares = movementMasks[piece][sq];
  if (is_sliding(piece)) {
    Bitboard blockers = allPieces & movementMasks[piece][sq];
    if (piece == W_ROOK || piece == B_ROOK)
      legal_squares = rookAttack[AttackKey{sq, blockers}];
    else if (piece == W_BISHOP || piece == B_BISHOP)
      legal_squares = bishopAttack[AttackKey{sq, blockers}];
    else if (piece == W_QUEEN || piece == B_QUEEN) {
      blockers = allPieces & movementMasks[W_BISHOP][sq];
      legal_squares = bishopAttack[AttackKey{sq, blockers}];

      blockers = allPieces & movementMasks[W_ROOK][sq];
      legal_squares |= rookAttack[AttackKey{sq, blockers}];
    }
  }

  if (piece == W_PAWN) {
    // white pawns can't move to 8th rank
    legal_squares &= 0x00FFFFFFFFFFFFFFULL;
    // can't move forward if square is occupied
    Bitboard occupied = allWhite | allBlack;
    legal_squares &= ~occupied;
    // can't do double pawn push if square in front is occupied
    if (sq / 8 == 1 && (occupied & (1ULL << (sq + 8)))) {
      legal_squares &= ~(1ULL << (sq + 16));
    }
  } else if (piece == B_PAWN) {
    // black pawns can't move to 1st rank
    legal_squares &= 0xFFFFFFFFFFFFFF00ULL;
    // can't move forward if square is occupied
    Bitboard occupied = allWhite | allBlack;
    legal_squares &= ~occupied;
    // can't do double pawn push if square in front is occupied
    if (sq / 8 == 6 && (occupied & (1ULL << (sq - 8)))) {
      legal_squares &= ~(1ULL << (sq - 16));
    }
  }

  return legal_squares;
}

/* returns legal castling squares for the king
 *
 * requirements for castling:
 * - not in check
 * - travelling squares between src and dst can't be attacked by enemy pieces
 * - can't castle through any pieces
 */
vector<Move> castling(const Position& pos, Bitboard enemy_attacks) {
  vector<Move> moves;

  if (pos.is_check()) return moves;

  Color color = pos.get_player();
  unsigned int castling_rights = pos.get_castling_rights();

  Bitboard kingside_clear_mask = color == WHITE ? kingsideCastleMaskW : kingsideCastleMaskB;
  Bitboard queenside_clear_mask = color == WHITE ? queensideCastleMaskW : queensideCastleMaskB;
  Bitboard queenside_safe_mask = color == WHITE ? 0x0CULL : (0x0CULL << 56);

  CastlingRights kingside = color == WHITE ? WK : BK;
  CastlingRights queenside = color == WHITE ? WQ : BQ;

  Square king_from = color == WHITE ? 4 : 60;
  Square kingside_to = color == WHITE ? 6 : 62;
  Square queenside_to = color == WHITE ? 2 : 58;
  Bitboard pieces = pos.all_pieces();

  bool is_king_castle_possible =
      (pieces & kingside_clear_mask) == 0 &&
      (enemy_attacks & kingside_clear_mask) == 0;
  bool is_queen_castle_possible =
      (pieces & queenside_clear_mask) == 0 &&
      (enemy_attacks & queenside_safe_mask) == 0;

  if ((castling_rights & kingside) && is_king_castle_possible) {
    Move move = {king_from, kingside_to};
    move.flags = static_cast<MoveFlags>(move.flags | KING_CASTLE);
    moves.push_back(move);
  }

  if ((castling_rights & queenside) && is_queen_castle_possible) {
    Move move = {king_from, queenside_to};
    move.flags = static_cast<MoveFlags>(move.flags | QUEEN_CASTLE);
    moves.push_back(move);
  }

  return moves;
}

std::vector<Move> bitboard_to_moves(Bitboard board, Square from) {
  std::vector<Move> moves;
  for (Square to = 0; to < 64; to++) {
    if (board & (1ULL << to)) {
      Move move = {from, to};
      moves.push_back(move);
    }
  }
  return moves;
}

Bitboard pawn_attacks(Color color, Square sq) {
  Bitboard attacks = 0;
  if (color == WHITE) {
    if (sq % 8 != 0 && sq + 7 < 64) // not a-file
      attacks |= (1ULL << (sq + 7));
    if (sq % 8 != 7 && sq + 9 < 64) // not h-file
      attacks |= (1ULL << (sq + 9));
  } else {
    if (sq % 8 != 0 && sq >= 9) // not a-file
      attacks |= (1ULL << (sq - 9));
    if (sq % 8 != 7 && sq >= 7) // not h-file
      attacks |= (1ULL << (sq - 7));
  }
  return attacks;
}

} // namespace

// returns bitboard of all squares attacked by the given player
Bitboard attack_mask(const Position& pos, Color player) {
  Bitboard attackSquares = 0;
  Bitboard allWhite = pos.all_pieces(WHITE);
  Bitboard allBlack = pos.all_pieces(BLACK);

  int start = player == WHITE ? W_PAWN : B_PAWN;
  int end = start + 6;
  for (int type = start; type < end; type++) {
    Bitboard pieces = pos.get_bitboard_of((PieceType) type);
    while (pieces) {
      Square sq = __builtin_ctzll(pieces);
      pieces &= pieces - 1; // clear lsb

      Bitboard legalSquares = 0;
      if (type == W_PAWN || type == B_PAWN) {
        legalSquares = pawn_attacks(player, sq);
      } else {
        legalSquares = pseudo_legal_moves((PieceType) type, sq, pos);
      }

      attackSquares |= legalSquares;
    }
  }

  return attackSquares;
}

} // namespace MoveGen
