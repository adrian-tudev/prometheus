#include "gui.h"

#include <algorithm>
#include <cmath>
#include <filesystem>

#include <SDL3_image/SDL_image.h>

using std::string;

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
  selected.reset();
  legalTargets = 0;
  lastMove.reset();

  pos = Position();
  pos.set(STARTING_FEN);
}

void GUI::undo() {
  if (!history.empty()) {
    pos = history.back();
    history.pop_back();
  }
  selected.reset();
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
  const float pad = 24.0f;
  const float minDim = (float) std::min(winW, winH);
  float boardSize = minDim - 2.0f * pad;
  boardSize = std::floor(boardSize / 8.0f) * 8.0f;
  boardSize = std::max(boardSize, 8.0f);
  g.tile = boardSize / 8.0f;
  g.board = SDL_FRect{
    (winW - boardSize) * 0.5f,
    (winH - boardSize) * 0.5f,
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
  PieceType p = pos.piece_on(sq);
  if (p == EMPTY) return false;
  Color toMove = pos.get_player();
  return (toMove == WHITE) ? piece_is_white(p) : !piece_is_white(p);
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
  const auto moves = movegen.generate_moves_at(sq, pos);
  legalTargets = moves_to_targets_mask(moves);
}

void GUI::handle_click(float x, float y) {
  const BoardGeom g = board_geom();
  auto sqOpt = square_from_mouse(x, y, g);
  if (!sqOpt) {
    selected.reset();
    legalTargets = 0;
    return;
  }

  const Square sq = *sqOpt;
  if (!selected) {
    if (is_own_piece(sq)) {
      select_square(sq);
    }
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
    Move mv;
    mv.from = from;
    mv.to = sq;

    history.push_back(pos);
    pos.do_move(mv);
    lastMove = mv;

    selected.reset();
    legalTargets = 0;
  } else {
    selected.reset();
    legalTargets = 0;
  }
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
        handle_click(e.button.x, e.button.y);
      } else if (e.button.button == SDL_BUTTON_RIGHT) {
        selected.reset();
        legalTargets = 0;
      }
      break;
    case SDL_EVENT_KEY_DOWN:
      if (e.key.repeat) break;
      if (e.key.scancode == SDL_SCANCODE_ESCAPE) {
        if (selected) {
          selected.reset();
          legalTargets = 0;
        } else {
          running = false;
        }
      } else if (e.key.scancode == SDL_SCANCODE_U) {
        undo();
      } else if (e.key.scancode == SDL_SCANCODE_R) {
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
    SDL_FRect r = rect_for_square(*selected, g);
    SDL_SetRenderDrawColor(renderer, 40, 160, 255, 220);
    SDL_RenderRect(renderer, &r);
    // make border thicker
    SDL_FRect r2{r.x + 1.0f, r.y + 1.0f, r.w - 2.0f, r.h - 2.0f};
    SDL_RenderRect(renderer, &r2);
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
        SDL_SetRenderDrawColor(renderer, 220, 30, 30, 160);
        SDL_RenderRect(renderer, &r);
        SDL_FRect r2{r.x + 2.0f, r.y + 2.0f, r.w - 4.0f, r.h - 4.0f};
        SDL_RenderRect(renderer, &r2);
      }
    }
  }
}

void GUI::render_pieces(const BoardGeom& g) {
  for (Square sq = 0; sq < 64; sq++) {
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
}

void GUI::render_hud() {
  SDL_SetRenderDrawColor(renderer, 20, 20, 20, 230);
  SDL_RenderDebugTextFormat(renderer, 12.0f, 12.0f, "to move: %s   [LMB] select/move  [RMB] clear  [U] undo  [R] reset  [Esc] back/quit", pos.get_player() == WHITE ? "white" : "black");
  if (!resRoot.empty()) {
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 200);
    SDL_RenderDebugTextFormat(renderer, 12.0f, 24.0f, "res: %s", resRoot.c_str());
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
  render_hud();

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
    SDL_Delay(1);
  }

  return 0;
}
