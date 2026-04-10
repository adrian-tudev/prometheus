#include "gui.h"

#include <algorithm>
#include <cmath>
#include <filesystem>

#include <SDL3_image/SDL_image.h>

using std::string;

constexpr float kDebugTextScale = 1.15f;
constexpr float kDebugPanelTop = 8.0f;
constexpr float kDebugPanelHeight = 44.0f;
constexpr float kDebugPanelGap = 8.0f;
constexpr float kBottomHudReserve = 42.0f;

static bool file_exists(const std::string& p) {
  std::error_code ec;
  return std::filesystem::exists(p, ec);
}

static std::string path_join(const std::string& a, const std::string& b) {
  if (a.empty()) return b;
  if (a.back() == '/') return a + b;
  return a + "/" + b;
}

static std::string path_parent(const std::string& p) {
  std::error_code ec;
  auto pp = std::filesystem::path(p);
  auto parent = pp.parent_path();
  return parent.empty() ? std::string() : parent.string();
}

static std::string normalize_dir(const std::string& p) {
  if (p.empty()) return p;
  if (p.back() == '/') return p;
  return p + "/";
}

static std::string move_flags_to_string(MoveFlags flags) {
  const unsigned int mask = static_cast<unsigned int>(flags);
  std::vector<std::string> names;
  if (mask & CAPTURE) names.push_back("CAPTURE");
  if (mask & DOUBLE_PAWN_PUSH) names.push_back("DOUBLE_PAWN_PUSH");
  if (mask & KING_CASTLE) names.push_back("KING_CASTLE");
  if (mask & QUEEN_CASTLE) names.push_back("QUEEN_CASTLE");
  if (mask & EN_PASSANT) names.push_back("EN_PASSANT");
  if (mask & PROMOTION) names.push_back("PROMOTION");
  if (names.empty()) return "none";

  std::string out;
  for (size_t i = 0; i < names.size(); i++) {
    if (i > 0) out += ",";
    out += names[i];
  }
  return out;
}

static std::string piece_to_string(PieceType p) {
  if (p == EMPTY) return "-";
  return std::string(1, charToPiece[p]);
}

static void render_debug_text_scaled(SDL_Renderer* renderer, float x, float y, const std::string& text, float scale) {
  if (!renderer) return;
  if (scale <= 1.0f) {
    SDL_RenderDebugText(renderer, x, y, text.c_str());
    return;
  }

  float prevX = 1.0f;
  float prevY = 1.0f;
  SDL_GetRenderScale(renderer, &prevX, &prevY);
  SDL_SetRenderScale(renderer, scale, scale);
  SDL_RenderDebugText(renderer, x / scale, y / scale, text.c_str());
  SDL_SetRenderScale(renderer, prevX, prevY);
}

GUI::GUI() {
  pieceTex.fill(nullptr);
}

GUI::~GUI() {
  unload_resources();
  if (renderer) SDL_DestroyRenderer(renderer);
  if (window) SDL_DestroyWindow(window);
  SDL_Quit();
}

bool GUI::init_sdl() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return false;
  }

  window = SDL_CreateWindow("prometheus gui", winW, winH, SDL_WINDOW_RESIZABLE);
  if (!window) {
    SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
    return false;
  }

  renderer = SDL_CreateRenderer(window, nullptr);
  if (!renderer) {
    SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
    return false;
  }

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderVSync(renderer, 1);
  update_window_size();
  return true;
}

static std::string resolve_res_root() {
  const char* base = SDL_GetBasePath();
  std::vector<std::string> candidates;

  if (base && *base) {
    std::string b = normalize_dir(base);
    // Common layout: repo/bin/prometheus-gui => base == ".../bin/"
    candidates.push_back(normalize_dir(path_join(b, "../res")));
    candidates.push_back(normalize_dir(path_join(b, "res")));
    candidates.push_back(normalize_dir(path_join(path_parent(b), "res")));
  }

  // Current working directory fallbacks
  candidates.push_back(normalize_dir("res"));
  candidates.push_back(normalize_dir("../res"));

  for (const auto& c : candidates) {
    if (file_exists(path_join(c, "white-king.png")) && file_exists(path_join(c, "black-king.png"))) {
      return c;
    }
  }

  return "";
}

bool GUI::load_resources() {
  resRoot = resolve_res_root();
  if (resRoot.empty()) {
    SDL_Log("Could not locate res/ folder. Tried relative to executable and cwd.");
    return false;
  }

  struct Entry { PieceType type; const char* file; };
  const Entry entries[] = {
    {W_PAWN, "white-pawn.png"},
    {W_KNIGHT, "white-knight.png"},
    {W_BISHOP, "white-bishop.png"},
    {W_ROOK, "white-rook.png"},
    {W_QUEEN, "white-queen.png"},
    {W_KING, "white-king.png"},
    {B_PAWN, "black-pawn.png"},
    {B_KNIGHT, "black-knight.png"},
    {B_BISHOP, "black-bishop.png"},
    {B_ROOK, "black-rook.png"},
    {B_QUEEN, "black-queen.png"},
    {B_KING, "black-king.png"},
  };

  for (auto& t : pieceTex) t = nullptr;

  for (const auto& e : entries) {
    const std::string path = path_join(resRoot, e.file);
    SDL_Texture* tex = IMG_LoadTexture(renderer, path.c_str());
    if (!tex) {
      SDL_Log("Failed to load %s: %s", path.c_str(), SDL_GetError());
      return false;
    }
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    pieceTex[e.type] = tex;
  }

  return true;
}

void GUI::unload_resources() {
  for (auto& tex : pieceTex) {
    if (tex) SDL_DestroyTexture(tex);
    tex = nullptr;
  }
}

void GUI::reset_position() {
  history.clear();
  redoHistory.clear();
  pendingPromotionMoves.clear();
  selected.reset();
  legalTargets = 0;
  lastMove.reset();
  engineMovePending = false;

  pos = Position();
  pos.set(STARTING_FEN);
}

void GUI::undo() {
  if (!history.empty()) {
    redoHistory.push_back(pos);
    pos = history.back();
    history.pop_back();
  }
  engineMovePending = false;
  selected.reset();
  pendingPromotionMoves.clear();
  legalTargets = 0;
  lastMove.reset();
}

void GUI::redo() {
  if (!redoHistory.empty()) {
    history.push_back(pos);
    pos = redoHistory.back();
    redoHistory.pop_back();
  }
  engineMovePending = false;
  selected.reset();
  pendingPromotionMoves.clear();
  legalTargets = 0;
  lastMove.reset();
}

void GUI::update_window_size() {
  int w = winW, h = winH;
  if (renderer) {
    SDL_GetRenderOutputSize(renderer, &w, &h);
  } else if (window) {
    SDL_GetWindowSizeInPixels(window, &w, &h);
  }
  winW = std::max(1, w);
  winH = std::max(1, h);
}

GUI::BoardGeom GUI::board_geom() const {
  BoardGeom g;
  const float sidePad = 24.0f;
  const float topReserve = kDebugPanelTop + kDebugPanelHeight + kDebugPanelGap;
  const float bottomReserve = kBottomHudReserve;
  const float availW = std::max(8.0f, (float) winW - 2.0f * sidePad);
  const float availH = std::max(8.0f, (float) winH - topReserve - bottomReserve);
  float boardSize = std::min(availW, availH);
  boardSize = std::floor(boardSize / 8.0f) * 8.0f;
  boardSize = std::max(boardSize, 8.0f);
  g.tile = boardSize / 8.0f;
  const float boardY = topReserve + std::max(0.0f, (availH - boardSize) * 0.5f);
  g.board = SDL_FRect{
    (winW - boardSize) * 0.5f,
    boardY,
    boardSize,
    boardSize,
  };
  return g;
}

bool GUI::point_in_rect(float x, float y, const SDL_FRect& r) const {
  return x >= r.x && y >= r.y && x < (r.x + r.w) && y < (r.y + r.h);
}

std::optional<Square> GUI::square_from_mouse(float x, float y, const BoardGeom& g) const {
  if (!point_in_rect(x, y, g.board)) return std::nullopt;
  const float bx = x - g.board.x;
  const float by = y - g.board.y;
  const int tx = (int) (bx / g.tile);
  const int ty = (int) (by / g.tile);
  if (tx < 0 || tx >= 8 || ty < 0 || ty >= 8) return std::nullopt;
  const int file = tx;
  const int rank = 7 - ty;
  return (Square) (rank * 8 + file);
}

SDL_FRect GUI::rect_for_square(Square sq, const BoardGeom& g) const {
  const int file = sq % 8;
  const int rank = sq / 8;
  const int tx = file;
  const int ty = 7 - rank;
  return SDL_FRect{
    g.board.x + tx * g.tile,
    g.board.y + ty * g.tile,
    g.tile,
    g.tile,
  };
}

bool GUI::is_own_piece(Square sq) const {
  if (!is_human_turn()) return false;
  PieceType p = pos.piece_on(sq);
  if (p == EMPTY) return false;
  Color toMove = pos.get_player();
  return (toMove == WHITE) ? piece_is_white(p) : !piece_is_white(p);
}

bool GUI::is_human_turn() const {
  return pos.get_player() == humanColor;
}

void GUI::play_engine_turn() {
  if (!engineMovePending) return;
  if (is_human_turn()) {
    engineMovePending = false;
    return;
  }
  if (!pendingPromotionMoves.empty()) return;

  if (MoveGen::generate_moves(pos).empty()) return;

  engine.set_position(pos);
  commit_move(engine.ponder());
  engineMovePending = false;
}

Bitboard GUI::moves_to_targets_mask(const std::vector<Move>& moves) const {
  Bitboard m = 0;
  for (const auto& mv : moves) {
    m |= (1ULL << mv.to);
  }
  return m;
}

void GUI::select_square(Square sq) {
  selected = sq;
  const auto moves = MoveGen::generate_moves_at(sq, pos);
  legalTargets = moves_to_targets_mask(moves);
}

void GUI::commit_move(Move mv) {
  if (mv.flags & EN_PASSANT) {
    PieceType movingPiece = pos.piece_on(mv.from);
    mv.captured_piece = piece_is_white(movingPiece) ? B_PAWN : W_PAWN;
  } else {
    mv.captured_piece = pos.piece_on(mv.to);
  }

  history.push_back(pos);
  redoHistory.clear();
  pos.do_move(mv);
  lastMove = mv;

  pendingPromotionMoves.clear();
  selected.reset();
  legalTargets = 0;

  if (!is_human_turn()) {
    engineMovePending = true;
  }
}

bool GUI::try_play_move(Square from, Square to) {
  Move mv{from, to};
  const auto legalMoves = MoveGen::generate_moves_at(from, pos);
  std::vector<Move> matching;
  for (const Move& m : legalMoves) {
    if (m.to == to) matching.push_back(m);
  }
  if (matching.empty()) return false;

  if (matching.size() > 1) {
    bool hasPromotion = false;
    for (const Move& m : matching) {
      if (m.flags & PROMOTION) {
        hasPromotion = true;
        break;
      }
    }
    if (hasPromotion) {
      pendingPromotionMoves = matching;
      return true;
    }
  }

  mv = matching.front();
  commit_move(mv);
  return true;
}

std::vector<SDL_FRect> GUI::promotion_option_rects(const BoardGeom& g) const {
  std::vector<SDL_FRect> rects;
  if (pendingPromotionMoves.empty()) return rects;

  const float box = std::max(40.0f, g.tile * 0.9f);
  const float gap = std::max(6.0f, g.tile * 0.08f);
  const float totalW = box * 4.0f + gap * 3.0f;
  const float x0 = g.board.x + (g.board.w - totalW) * 0.5f;
  const float y0 = g.board.y + (g.board.h - box) * 0.5f;
  for (int i = 0; i < 4; i++) {
    rects.push_back(SDL_FRect{x0 + i * (box + gap), y0, box, box});
  }
  return rects;
}

bool GUI::handle_promotion_click(float x, float y) {
  if (pendingPromotionMoves.empty()) return false;
  const BoardGeom g = board_geom();
  const auto rects = promotion_option_rects(g);
  if (rects.size() != 4) return false;

  PieceType promoOrder[4] = {
    pos.get_player() == WHITE ? W_QUEEN : B_QUEEN,
    pos.get_player() == WHITE ? W_ROOK : B_ROOK,
    pos.get_player() == WHITE ? W_BISHOP : B_BISHOP,
    pos.get_player() == WHITE ? W_KNIGHT : B_KNIGHT,
  };

  for (int i = 0; i < 4; i++) {
    if (!point_in_rect(x, y, rects[i])) continue;
    for (const Move& m : pendingPromotionMoves) {
      if (m.promotion == promoOrder[i]) {
        commit_move(m);
        return true;
      }
    }
    return true;
  }

  pendingPromotionMoves.clear();
  return true;
}

void GUI::handle_click(float x, float y) {
  if (handle_promotion_click(x, y)) return;

  const BoardGeom g = board_geom();
  auto sqOpt = square_from_mouse(x, y, g);
  if (!sqOpt) {
    selected.reset();
    legalTargets = 0;
    return;
  }

  const Square sq = *sqOpt;

  // If nothing is selected, only own pieces become selected.
  if (!selected) {
    if (is_own_piece(sq)) select_square(sq);
    return;
  }

  const Square from = *selected;

  // click same square toggles selection off
  if (sq == from) {
    selected.reset();
    legalTargets = 0;
    return;
  }

  // If clicking another own piece, switch selection.
  if (is_own_piece(sq)) {
    select_square(sq);
    return;
  }

  // Otherwise attempt to play if it's a legal target.
  if (legalTargets & (1ULL << sq)) {
    if (!try_play_move(from, sq)) {
      selected.reset();
      legalTargets = 0;
    }
  } else {
    // Click on a non-legal square (including empty squares) clears selection.
    selected.reset();
    legalTargets = 0;
  }
}

void GUI::start_drag(Square sq, float x, float y) {
  dragArmed = true;
  dragging = false;
  dragWasSelectedOnDown = (selected && *selected == sq);
  dragFrom = sq;
  dragPiece = pos.piece_on(sq);
  dragStartX = x;
  dragStartY = y;
  dragX = x;
  dragY = y;

  // Show selection/move targets immediately on mouse down.
  select_square(sq);
}

void GUI::update_drag(float x, float y) {
  if (!dragArmed) return;
  dragX = x;
  dragY = y;

  if (!dragging) {
    const float dx = x - dragStartX;
    const float dy = y - dragStartY;
    // Small threshold to avoid starting a drag on tiny hand jitter.
    if ((dx * dx + dy * dy) >= 16.0f) dragging = true;
  }
}

void GUI::cancel_drag() {
  dragArmed = false;
  dragging = false;
  dragWasSelectedOnDown = false;
  dragPiece = EMPTY;
}

void GUI::finish_drag(float x, float y) {
  if (handle_promotion_click(x, y)) {
    cancel_drag();
    return;
  }

  if (!dragArmed) return;

  // Treat mouse-up without a real drag as a click on the original square.
  if (!dragging) {
    // If the piece was already selected, a click toggles it off.
    if (dragWasSelectedOnDown) {
      selected.reset();
      legalTargets = 0;
    }
    cancel_drag();
    return;
  }

  const BoardGeom g = board_geom();
  auto dropOpt = square_from_mouse(x, y, g);
  if (!dropOpt) {
    // Dropped outside the board: keep selection on the dragged piece.
    cancel_drag();
    return;
  }

  const Square to = *dropOpt;
  if (to != dragFrom && (legalTargets & (1ULL << to))) {
    if (!try_play_move(dragFrom, to)) {
      selected.reset();
      legalTargets = 0;
    }
  } else if (to != dragFrom && is_own_piece(to)) {
    select_square(to);
  }

  cancel_drag();
}

void GUI::handle_event(const SDL_Event& e, bool& running) {
  switch (e.type) {
    case SDL_EVENT_QUIT:
      running = false;
      break;
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
      update_window_size();
      break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      if (e.button.button == SDL_BUTTON_LEFT) {
        const BoardGeom g = board_geom();
        auto sqOpt = square_from_mouse(e.button.x, e.button.y, g);
        if (sqOpt && is_own_piece(*sqOpt)) {
          start_drag(*sqOpt, e.button.x, e.button.y);
        } else {
          cancel_drag();
          handle_click(e.button.x, e.button.y);
        }
      } else if (e.button.button == SDL_BUTTON_RIGHT) {
        cancel_drag();
        pendingPromotionMoves.clear();
        selected.reset();
        legalTargets = 0;
      }
      break;
    case SDL_EVENT_MOUSE_MOTION:
      update_drag(e.motion.x, e.motion.y);
      break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
      if (e.button.button == SDL_BUTTON_LEFT) {
        finish_drag(e.button.x, e.button.y);
      }
      break;
    case SDL_EVENT_KEY_DOWN:
      if (e.key.repeat) break;
      if (e.key.scancode == SDL_SCANCODE_ESCAPE) {
        cancel_drag();
        if (!pendingPromotionMoves.empty()) {
          pendingPromotionMoves.clear();
          break;
        }
        if (selected) {
          selected.reset();
          legalTargets = 0;
        } else {
          running = false;
        }
      } else if (e.key.scancode == SDL_SCANCODE_LEFT) {
        cancel_drag();
        undo();
      } else if (e.key.scancode == SDL_SCANCODE_RIGHT) {
        cancel_drag();
        redo();
      } else if (e.key.scancode == SDL_SCANCODE_R) {
        cancel_drag();
        reset_position();
      }
      break;
    default:
      break;
  }
}

void GUI::render_board(const BoardGeom& g) {
  const SDL_Color light{240, 231, 214, 255};
  const SDL_Color dark{172, 136, 102, 255};

  for (int ty = 0; ty < 8; ty++) {
    for (int tx = 0; tx < 8; tx++) {
      SDL_FRect r{
        g.board.x + tx * g.tile,
        g.board.y + ty * g.tile,
        g.tile,
        g.tile,
      };
      const bool isDark = ((tx + ty) & 1) == 1;
      const SDL_Color c = isDark ? dark : light;
      SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
      SDL_RenderFillRect(renderer, &r);
    }
  }
}

void GUI::render_highlights(const BoardGeom& g) {
  // last move highlight
  if (lastMove) {
    const SDL_Color c{255, 214, 10, 70};
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_FRect rf = rect_for_square(lastMove->from, g);
    SDL_FRect rt = rect_for_square(lastMove->to, g);
    SDL_RenderFillRect(renderer, &rf);
    SDL_RenderFillRect(renderer, &rt);
  }

  // selected highlight
  if (selected) {
    // Don't show the blue selection fill for empty-square selection.
    if (pos.piece_on(*selected) != EMPTY) {
      SDL_FRect r = rect_for_square(*selected, g);
      SDL_SetRenderDrawColor(renderer, 40, 160, 255, 110);
      SDL_RenderFillRect(renderer, &r);
    }
  }

  // legal target highlights
  if (legalTargets) {
    for (Square sq = 0; sq < 64; sq++) {
      if (!(legalTargets & (1ULL << sq))) continue;

      SDL_FRect r = rect_for_square(sq, g);
      PieceType p = pos.piece_on(sq);

      if (p == EMPTY) {
        SDL_FRect dot{
          r.x + r.w * 0.40f,
          r.y + r.h * 0.40f,
          r.w * 0.20f,
          r.h * 0.20f,
        };
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 90);
        SDL_RenderFillRect(renderer, &dot);
      } else {
        // Capture target: tint the whole square.
        SDL_SetRenderDrawColor(renderer, 220, 30, 30, 70);
        SDL_RenderFillRect(renderer, &r);
      }
    }
  }
}

void GUI::render_pieces(const BoardGeom& g) {
  for (Square sq = 0; sq < 64; sq++) {
    if (dragging && dragPiece != EMPTY && sq == dragFrom) continue;

    PieceType p = pos.piece_on(sq);
    if (p == EMPTY) continue;
    SDL_Texture* tex = pieceTex[p];
    if (!tex) continue;

    SDL_FRect r = rect_for_square(sq, g);
    // Slight padding so pieces don't touch edges.
    const float pad = std::max(2.0f, r.w * 0.08f);
    SDL_FRect dst{r.x + pad, r.y + pad, r.w - 2 * pad, r.h - 2 * pad};
    SDL_RenderTexture(renderer, tex, nullptr, &dst);
  }

  // Draw dragged piece on top, following the cursor.
  if (dragging && dragPiece != EMPTY) {
    SDL_Texture* tex = pieceTex[dragPiece];
    if (tex) {
      SDL_FRect r{
        dragX - g.tile * 0.5f,
        dragY - g.tile * 0.5f,
        g.tile,
        g.tile,
      };
      const float pad = std::max(2.0f, r.w * 0.08f);
      SDL_FRect dst{r.x + pad, r.y + pad, r.w - 2 * pad, r.h - 2 * pad};
      SDL_RenderTexture(renderer, tex, nullptr, &dst);
    }
  }
}

void GUI::render_hud() {
  SDL_SetRenderDrawColor(renderer, 20, 20, 20, 230);
  const std::string toMove = (pos.get_player() == WHITE) ? "white" : "black";
  const std::string humanSide = (humanColor == WHITE) ? "white" : "black";
  const std::string turnOwner = is_human_turn() ? "you" : "engine";
  const bool isCheckmate = pos.is_check_mate();
  const std::string status = isCheckmate ? "   [CHECKMATE]" : "";
  const std::string promoHint = pendingPromotionMoves.empty() ? "" : "   [PROMOTE: choose Q/R/B/N]";
  const std::string hud =
      std::string("mode: vs-engine") +
      "  you: " + humanSide +
      "  to move: " + toMove + " (" + turnOwner + ")" +
      status + promoHint +
      "   [LMB] select/move  [RMB] clear  [<-] undo  [->] redo  [R] reset  [Esc] back/quit";
  float y = (float) winH - 30.0f;
  render_debug_text_scaled(renderer, 12.0f, y, hud, kDebugTextScale);
  if (!resRoot.empty()) {
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 200);
    const std::string resLine = "res: " + resRoot;
    render_debug_text_scaled(renderer, 12.0f, y - 14.0f, resLine, kDebugTextScale);
  }
}

void GUI::render_state_window() {
  const BoardGeom g = board_geom();
  const State& s = pos.get_state();
  const bool inCheck = pos.is_check();
  const bool inCheckmate = pos.is_check_mate();

  std::string castling;
  if (s.castling_rights & CastlingRights::WK) castling += 'K';
  if (s.castling_rights & CastlingRights::WQ) castling += 'Q';
  if (s.castling_rights & CastlingRights::BK) castling += 'k';
  if (s.castling_rights & CastlingRights::BQ) castling += 'q';
  if (castling.empty()) castling = "-";

  std::string ep = "-";
  if (s.enPassant) {
    int epSq = __builtin_ctzll(s.enPassant);
    char file = (char) ('a' + (epSq % 8));
    char rank = (char) ('1' + (epSq / 8));
    ep = std::string() + file + rank;
  }

  const float panelPad = std::max(6.0f, g.tile * 0.10f);
  const float panelW = std::max(120.0f, g.board.w - panelPad * 2.0f);
  const float panelH = kDebugPanelHeight;
  const float panelY = kDebugPanelTop;
  const SDL_FRect panel{
    g.board.x + panelPad,
    panelY,
    panelW,
    panelH,
  };

  SDL_SetRenderDrawColor(renderer, 12, 12, 14, 215);
  SDL_RenderFillRect(renderer, &panel);

  SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
  SDL_RenderRect(renderer, &panel);

  const float x = panel.x + 8.0f;
  float y = panel.y + 6.0f;
  const float lh = 13.0f;

  SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
  const std::string stateLine =
      std::string("state  white=") + (s.white ? "true" : "false") +
      "  check=" + (inCheck ? std::string("true") : std::string("false")) +
      "  checkmate=" + (inCheckmate ? std::string("true") : std::string("false")) +
      "  castling=" + castling +
      "  ep=" + ep +
      "  rule50=" + std::to_string(s.rule50) +
      "  full=" + std::to_string(s.fullMoves);
  render_debug_text_scaled(renderer, x, y, stateLine, kDebugTextScale);
  y += lh;

  if (!lastMove) {
    render_debug_text_scaled(renderer, x, y, "last_move  none", kDebugTextScale);
    return;
  }

  const std::string moveLine =
      "last_move  from=" + format(lastMove->from) +
      "  to=" + format(lastMove->to) +
      "  move=" + format(*lastMove) +
      "  flags=" + move_flags_to_string(lastMove->flags) +
      "  captured=" + piece_to_string(lastMove->captured_piece) +
      "  promo=" + piece_to_string(lastMove->promotion);
  render_debug_text_scaled(renderer, x, y, moveLine, kDebugTextScale);
}

void GUI::render_promotion_picker(const BoardGeom& g) {
  if (pendingPromotionMoves.empty()) return;

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 110);
  SDL_FRect scr{0.0f, 0.0f, (float) winW, (float) winH};
  SDL_RenderFillRect(renderer, &scr);

  const auto rects = promotion_option_rects(g);
  PieceType promoOrder[4] = {
    pos.get_player() == WHITE ? W_QUEEN : B_QUEEN,
    pos.get_player() == WHITE ? W_ROOK : B_ROOK,
    pos.get_player() == WHITE ? W_BISHOP : B_BISHOP,
    pos.get_player() == WHITE ? W_KNIGHT : B_KNIGHT,
  };

  for (int i = 0; i < 4 && i < (int) rects.size(); i++) {
    SDL_SetRenderDrawColor(renderer, 235, 235, 235, 250);
    SDL_RenderFillRect(renderer, &rects[i]);
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderRect(renderer, &rects[i]);

    SDL_Texture* tex = pieceTex[promoOrder[i]];
    if (!tex) continue;

    const float pad = std::max(2.0f, rects[i].w * 0.10f);
    SDL_FRect dst{rects[i].x + pad, rects[i].y + pad, rects[i].w - 2 * pad, rects[i].h - 2 * pad};
    SDL_RenderTexture(renderer, tex, nullptr, &dst);
  }
}

void GUI::render() {
  update_window_size();
  const BoardGeom g = board_geom();

  SDL_SetRenderDrawColor(renderer, 18, 18, 22, 255);
  SDL_RenderClear(renderer);

  render_board(g);
  render_highlights(g);
  render_pieces(g);
  render_promotion_picker(g);
  render_hud();
  render_state_window();

  SDL_RenderPresent(renderer);
}

int GUI::run() {
  Bitboards::init();

  if (!init_sdl()) return 1;
  if (!load_resources()) return 1;
  reset_position();

  bool running = true;
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      handle_event(e, running);
    }
    render();
    play_engine_turn();
    SDL_Delay(1);
  }

  return 0;
}
