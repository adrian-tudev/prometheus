#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <iostream>
#include <string>

#include "types.h"

const std::string charToPiece = "pnbrqkPNBRQK ";
const std::string files = "abcdefgh";
const std::string ranks = "12345678";

constexpr int32_t MAX_EVAL = 1e5;
const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

// formats a move to algebraic notation
std::string format(Move move);
std::string format(Square sq);

// parses a move in algebraic notation
Move parse_move(const std::string& str);

std::string format(Square sq);
bool is_sliding(PieceType piece);
Color get_color(PieceType piece);

#endif
