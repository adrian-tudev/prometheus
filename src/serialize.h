#pragma once

#include <fstream>
#include <unordered_map>

#include "bitboard.h"

using namespace Bitboards;
using std::unordered_map;

void save(const unordered_map<AttackKey, Bitboard, AttackKeyHash>& map, const std::string& filename);
void load(const std::string& filename);

