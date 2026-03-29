#ifndef PROMETHEUS_GUI_H
#define PROMETHEUS_GUI_H

#include <array>
#include <optional>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "bitboard.h"
#include "movegen.h"
#include "position.h"
#include "types.h"
#include "utils.h"

class GUI {
public:
  GUI();
  ~GUI();

  int run();

private:
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;

  std::array<SDL_Texture*, pieceTypes> pieceTex{};

  MoveGen movegen;
  std::vector<Position> history;
  Position pos;

  std::optional<Square> selected;
  Bitboard legalTargets = 0;
  std::optional<Move> lastMove;

  int winW = 900;
  int winH = 900;

  std::string resRoot;

  bool init_sdl();
  bool load_resources();
  void unload_resources();

  void reset_position();
  void undo();

  void handle_event(const SDL_Event& e, bool& running);
  void handle_click(float x, float y);
  void update_window_size();

  struct BoardGeom {
    SDL_FRect board;
    float tile = 0.0f;
  };

  BoardGeom board_geom() const;
  bool point_in_rect(float x, float y, const SDL_FRect& r) const;

  std::optional<Square> square_from_mouse(float x, float y, const BoardGeom& g) const;
  SDL_FRect rect_for_square(Square sq, const BoardGeom& g) const;

  void select_square(Square sq);
  bool is_own_piece(Square sq) const;

  Bitboard moves_to_targets_mask(const std::vector<Move>& moves) const;

  void render();
  void render_board(const BoardGeom& g);
  void render_highlights(const BoardGeom& g);
  void render_pieces(const BoardGeom& g);
  void render_hud();
};

#endif
