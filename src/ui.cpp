#include "ui.h"

UI::UI() {
  printf("\033[32mprometheus v.%s\033[0m\n", PROMETHEUS_VERSION);
  Bitboards::init();
  position.set(STARTING_FEN);
}

std::string readInput() {
  std::string line;
  std::getline(std::cin, line);
  return line;
}

void UI::loop() {
  while (true) {
    std::cout << "> ";
    std::string line = readInput();
    if (line == "quit" || line == "q") break;
    Move move = parse_move(line);

    // check if move is valid
    Color player = position.get_player();
    PieceType piece = position.piece_on(move.from);
    std::cout << "move: " << format(move.from) << " -> " << format(move.to) << "\n";
    if (piece == EMPTY) {
      printf("no piece at %s\n", format(move.from).c_str());
      continue;
    } else if ((player == WHITE && !piece_is_white(piece)) ||
               (player == BLACK && piece_is_white(piece))) {
      printf("not your piece!\n");
      continue;
    }  
    std::vector<Move> legalMoves = movegen.generate_moves_at(move.from, position);
    auto it = std::find(legalMoves.begin(), legalMoves.end(), move);
    if (it == legalMoves.end()) {
      printf("illegal move!\n");
      continue;
    }
    position.do_move(move);
    position.print();
  }
}
