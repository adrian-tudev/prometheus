#include "tt.h"

#include <algorithm>

TT::TT(size_t mb) {
  resize(mb);
}

void TT::resize(size_t mb) {
  const size_t bytes = std::max<size_t>(mb, 1) * 1024ULL * 1024ULL;
  size_t entries = std::max<size_t>(2, bytes / sizeof(TTEntry));

  size_t pow2 = 1;
  while ((pow2 << 1) <= entries) {
    pow2 <<= 1;
  }

  table.assign(pow2, TTEntry{});
  mask = pow2 - 1;
  age = 0;
}

void TT::clear() {
  for (auto& e : table) {
    e = TTEntry{};
  }
  age = 0;
}

void TT::new_search() {
  age++;
}

const TTEntry* TT::probe(Key key) const {
  if (table.empty()) return nullptr;

  const TTEntry& e = table[key & mask];
  if (e.flag == TT_NONE || e.key != key) return nullptr;
  return &e;
}

void TT::store(Key key, Score score, uint8_t depth, TTFlag flag, TTMove move) {
  if (table.empty()) return;

  TTEntry& e = table[key & mask];
  const bool empty_slot = (e.flag == TT_NONE);
  const bool same_key = (e.key == key);
  const bool old_gen = (e.age != age);
  const bool deeper_or_equal = (depth >= e.depth);

  if (!(empty_slot || same_key || old_gen || deeper_or_equal)) return;

  e.key = key;
  e.score = score;
  e.depth = depth;
  e.flag = flag;
  e.move = move;
  e.age = age;
}
