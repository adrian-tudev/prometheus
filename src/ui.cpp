#include "ui.h"

UI::UI() : engine(position) {
  printf("\033[32mprometheus v.%s\033[0m\n", PROMETHEUS_VERSION);
  Bitboards::init();
  position.set(STARTING_FEN);
  position.print();
}

std::string read_input() {
  std::string line;
  std::getline(std::cin, line);
  return line;
}

bool UI::valid_player(Move move) {
  // check if move is valid
  Color player = position.get_player();
  PieceType piece = position.piece_on(move.from);
  // std::cout << "move: " << format(move.from) << " -> " << format(move.to) << "\n";
  if (piece == EMPTY) {
    printf("no piece at %s\n", format(move.from).c_str());
    return false;
  } else if ((player == WHITE && !piece_is_white(piece)) ||
              (player == BLACK && piece_is_white(piece))) {
    printf("not your piece!\n");
    return false;
  }  
  return true;
}

bool UI::valid_move(Move move) {
  std::vector<Move> legalMoves = movegen.generate_moves_at(move.from, position);
  auto it = std::find(legalMoves.begin(), legalMoves.end(), move);
  if (it == legalMoves.end()) {
    printf("illegal move!\n");
    return false;
  }
  return true;
}

void UI::loop() {
  while (true) {
    std::cout << "> ";
    std::string line = read_input();
    if (line == "quit" || line == "q") break;
    Move move = parse_move(line);
    // has chosen valid piece
    if (!valid_player(move)) continue;
    // is valid move for that piece
    if (!valid_move(move)) continue;

    position.do_move(move);
    position.print();

    Score score = engine.eval();
  }
}