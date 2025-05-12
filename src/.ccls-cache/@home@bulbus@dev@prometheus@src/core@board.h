#include <stdint.h>
#include <string>
#include <vector>

namespace Game {

  class Board {
  public:
    Board() = default;
    Board(const std::string& FEN);
    ~Board();
    
    uint32_t eval();
    bool check();
    bool checkmate();
    void show();

  private:
    const std::string m_FEN;
  };
}
