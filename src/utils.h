#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>

#include "types.h"

using std::optional;
using std::string;

const string charToPiece = "pnbrqkPNBRQK ";
const string files = "abcdefgh";
const string ranks = "12345678";

constexpr int32_t MAX_EVAL = 1e5;
const string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// formats a move to algebraic notation
string format(Move move);
string format(Square sq);

// parses a move in algebraic notation
optional<Move> parse_move(const std::string& str);

string format(Square sq);
bool is_sliding(PieceType piece);
Color get_color(PieceType piece);

#endif
