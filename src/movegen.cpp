#include "movegen.h"

using namespace Bitboards;

std::vector<Move> MoveGen::generate_moves(const Position& pos) {
  std::vector<Move> moves;
  Color player = pos.get_player();
  Bitboard pieces = pos.all_pieces(player);

  while (pieces) {
    Square sq = __builtin_ctzll(pieces);
    pieces &= pieces - 1; // clear lsb

    std::vector<Move> pieceMoves = generate_moves_at(sq, pos);
    moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
  }

  return moves;
}

std::vector<Move> MoveGen::generate_moves_at(Square sq, const Position& pos) {
  PieceType piece = pos.piece_on(sq);
  assert(piece != PieceType::EMPTY);
  // legal squares for the piece on sq
  Bitboard legalSquares = pseudo_legal_moves(piece, sq, pos);

  // attacks for pawns (different from movement)
  if (piece == W_PAWN || piece == B_PAWN) {
    Bitboard capturable = pos.all_pieces(piece_is_white(piece) ? BLACK : WHITE);
    // TODO make a separate bitboard for captures to enable capture flag
    legalSquares |= capturable & pawn_attacks(get_color(piece), sq);
  }

  // checks   
  Bitboard enemyAttack = attack_mask(pos);
  if (piece == W_KING || piece == B_KING) {
    Bitboard enemyAttack = attack_mask(pos);
    if (pos.get_bitboard_of(piece) & enemyAttack) {
      // king is in check, can only move to squares not attacked by enemy
      legalSquares &= ~enemyAttack;
    }
  }

  // castling
  if (piece == W_KING || piece == B_KING) {
    legalSquares |= castling(pos);
  }

  // can't eat own pieces
  // important: do this after checking for checks to disable the king from capturing a "defended" piece
  Bitboard occupied = piece_is_white(piece) ? pos.all_pieces(WHITE) : pos.all_pieces(BLACK);
  legalSquares &= ~occupied;

  // pins

  // Bitboard to Moves
  std::vector<Move> moves = bitboard_to_moves(legalSquares, sq);

  return moves;
}

Bitboard MoveGen::pseudo_legal_moves(PieceType piece, Square sq, const Position& pos) {
  Bitboard allWhite = pos.all_pieces(WHITE);
  Bitboard allBlack = pos.all_pieces(BLACK);
  Bitboard allPieces = allWhite | allBlack;
  
  Bitboard legalSquares = movementMasks[piece][sq];
  if (is_sliding(piece)) {
    Bitboard blockers = allPieces & movementMasks[piece][sq];
    if (piece == W_ROOK || piece == B_ROOK)
      legalSquares = rookAttack[{sq, blockers}];
    else if (piece == W_BISHOP || piece == B_BISHOP)
      legalSquares = bishopAttack[{sq, blockers}];
    else if (piece == W_QUEEN || piece == B_QUEEN) {
      blockers = allPieces & movementMasks[W_BISHOP][sq];
      legalSquares = bishopAttack[{sq, blockers}];

      blockers = allPieces & movementMasks[W_ROOK][sq];
      legalSquares |= rookAttack[{sq, blockers}];
    }
  }

  if (piece == W_PAWN) {
    // white pawns can't move to 8th rank
    legalSquares &= 0x00FFFFFFFFFFFF;
    // can't move forward if square is occupied
    Bitboard occupied = allWhite | allBlack;
    legalSquares &= ~occupied;
    // can't do double pawn push if square in front is occupied
    if (sq / 8 == 1 && (occupied & (1ULL << (sq + 8)))) {
      legalSquares &= ~(1ULL << (sq + 16));
    }
  } else if (piece == B_PAWN) {
    // black pawns can't move to 1st rank
    legalSquares &= 0xFFFFFFFFFFFF00;
    // can't move forward if square is occupied
    Bitboard occupied = allWhite | allBlack;
    legalSquares &= ~occupied;
    // can't do double pawn push if square in front is occupied
    if (sq / 8 == 6 && (occupied & (1ULL << (sq - 8)))) {
      legalSquares &= ~(1ULL << (sq - 16));
    }
  }

  return legalSquares;
}

// returns bitboard of legal squares the king can move to when castling
// 0 if not available
Bitboard MoveGen::castling(const Position& pos) {
  Bitboard legalSquares = 0;
  // TODO

  return legalSquares;
}

std::vector<Move> MoveGen::bitboard_to_moves(Bitboard board, Square from) {
  std::vector<Move> moves;
  for (Square to = 0; to < 64; to++) {
    if (board & (1ULL << to)) {
      Move move = {from, to};
      moves.push_back(move);
    }
  }
  return moves;
}

Bitboard MoveGen::pawn_attacks(Color color, Square sq) {
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

// returns bitboard of all squares attacked by 'attacker' 
Bitboard MoveGen::attack_mask(const Position& pos) {
  Color attacker = pos.get_player() == WHITE ? BLACK : WHITE;

  Bitboard attackSquares = 0;
  Bitboard allWhite = pos.all_pieces(WHITE);
  Bitboard allBlack = pos.all_pieces(BLACK);

  int start = attacker == WHITE ? W_PAWN : B_PAWN;
  int end = start + 6;
  for (int type = start; type < end; type++) {
    Bitboard pieces = pos.get_bitboard_of((PieceType) type);
    while (pieces) {
      Square sq = __builtin_ctzll(pieces);
      pieces &= pieces - 1; // clear lsb

      Bitboard legalSquares = 0;
      if (type == W_PAWN || type == B_PAWN) {
        legalSquares = pawn_attacks(attacker, sq);
      } else {
        legalSquares = pseudo_legal_moves((PieceType) type, sq, pos);
      }

      attackSquares |= legalSquares;
    }
  }

  return attackSquares;
}

