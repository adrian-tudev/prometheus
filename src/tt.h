#pragma once

#include "types.h"

#include <cstddef>
#include <cstdint>
#include <vector>

using std::size_t;

using TTMove = uint16_t;
constexpr TTMove TT_MOVE_NONE = 0xFFFF;

inline TTMove pack_tt_move(const Move& m) {
  return static_cast<TTMove>(
      (static_cast<uint16_t>(m.from) & 0x3F) |
      ((static_cast<uint16_t>(m.to) & 0x3F) << 6) |
      ((static_cast<uint16_t>(m.promotion) & 0x0F) << 12));
}

inline Move unpack_tt_move(TTMove pm) {
  if (pm == TT_MOVE_NONE) return Move{};

  Move m{};
  m.from = static_cast<Square>(pm & 0x3F);
  m.to = static_cast<Square>((pm >> 6) & 0x3F);
  m.promotion = static_cast<PieceType>((pm >> 12) & 0x0F);
  return m;
}

enum TTFlag : uint8_t {
  TT_NONE,
  TT_EXACT,
  TT_LOWER,  // beta cutoff (lower bound)
  TT_HIGHER, // alpha cutoff (upper bound)
};

struct TTEntry {
  Key key = 0;
  Score score = 0;
  TTMove move = TT_MOVE_NONE;
  uint8_t depth = 0;
  TTFlag flag = TT_NONE;
  uint8_t age = 0;
};

class TT {
public:
  explicit TT(size_t mb = 64);

  void resize(size_t mb);
  void clear();
  void new_search();

  const TTEntry* probe(Key key) const;
  void store(Key key, Score score, uint8_t depth, TTFlag flag, TTMove move);

private:
  size_t mask = 0;
  uint8_t age = 0;
  std::vector<TTEntry> table;
};

