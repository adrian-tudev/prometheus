#include "ui.h"

UI::UI() {
  printf("\033[32mprometheus v.%s\033[0m\n", PROMETHEUS_VERSION);
  Bitboards::init();

  Position initPos;
  initPos.set(STARTING_FEN);
  positions.push_back(initPos);
}

std::string read_input() {
  string line;
  std::getline(std::cin, line);
  return line;
}

// check if selected piece is valid
bool UI::is_own_piece(Move move) {
  Color player = positions.back().get_player();
  PieceType piece = positions.back().piece_on(move.from);
  if (piece == EMPTY) {
    // printf("no piece at %s\n", format(move.from).c_str());
    return false;
  } else if ((player == WHITE && !piece_is_white(piece)) ||
              (player == BLACK && piece_is_white(piece))) {
    printf("not your piece!\n");
    return false;
  }  
  return true;
}

bool UI::is_move_legal(Move move) {
  std::vector<Move> legalMoves = movegen.generate_moves_at(move.from, positions.back());
  auto it = std::find(legalMoves.begin(), legalMoves.end(), move);
  if (it == legalMoves.end()) {
    printf("illegal move!\n");
    return false;
  }
  return true;
}

void UI::loop() {
  positions.back().print();
  while (true) {
    engine.set_position(positions.back());
    std::cout << "> ";
    string line = read_input();
    if (line == "quit" || line == "q") break;
    if (line == "undo" || line == "u") {
      if (positions.size() > 1) {
        positions.pop_back();
        positions.back().print();
      } else {
        printf("no moves to undo!\n");
      }
      continue;
    }
    if (line == "eval" || line == "e") {
      Score score = engine.eval();
      printf("eval: %d\n", score);
      continue;
    }
    // set position from FEN
    if (line == "fen" || line == "f") {
      std::cout << "enter FEN: ";
      string fen = read_input();
      Position pos;
      pos.set(fen);
      pos.print();
      positions.push_back(pos);
      continue;
    }

    if (auto move = parse_move(line)) {
      // has chosen valid piece
      if (!is_own_piece(*move)) continue;
      // is valid move for that piece
      if (!is_move_legal(*move)) continue;

      Position pos = positions.back();
      pos.do_move(*move);
      pos.print();
      positions.push_back(pos);

      Score score = engine.eval();
    } else {
      std::cout << "Not valid move!" << std::endl;
      continue;
    }
  }
}
