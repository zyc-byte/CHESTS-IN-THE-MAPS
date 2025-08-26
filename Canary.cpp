#include <algorithm>
#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include <conio.h>

// ƽ̨��غ���
namespace Platform {
inline void clearScreen() noexcept {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

inline void setColor() noexcept {
#ifdef _WIN32
  system("color 03");
#endif
}

inline void pause(std::chrono::milliseconds ms) noexcept {
  std::this_thread::sleep_for(ms);
}

inline char getChar() noexcept {
  return static_cast<char>(getch());
}
}  // namespace Platform

// ��Ϸ�����ռ�
namespace ChestsInMaps {

// ==================== �������� ====================
inline constexpr int MAX_MAP_SIZE    = 55;
inline constexpr int MOVE_DIRECTIONS = 4;
inline constexpr int GHOST_MAX_HP    = 3;
inline constexpr int PLAYER_DAMAGE   = 1;
inline constexpr int INITIAL_HEALTH  = 20;
inline constexpr int ITEM_TYPES      = 8;

// ǿ����ö�� - ��ͼԪ��
enum class MapElement : std::uint8_t {
  Empty     = 0,
  WallStart = 5,
  Wall      = 5,
  Wall2     = 6,
  Wall3     = 7,
  Chest     = 8,
  Ghost     = 9,
  Player    = 10,
  Door      = 11,
  GhostHurt = 12,
  GhostWeak = 13
};

// ��Ʒϵͳ
struct Item {
  std::string_view name;
  int value;
};

// ��Ʒ����
inline constexpr std::array<Item, ITEM_TYPES> ITEMS = {{{"ԭʯ", 1},
                                                        {"ú̿", 5},
                                                        {"����", 100},
                                                        {"��ʯ", 150},
                                                        {"���ʯ", 1000},
                                                        {"�̱�ʯ", 1000},
                                                        {"��", 100000},
                                                        {"��ʯ", 1000000}}};

// ��������
struct Direction {
  int dx, dy;
};

inline constexpr std::array<Direction, MOVE_DIRECTIONS> DIRECTIONS = {{
    {0, 1},   // ��
    {1, 0},   // ��
    {0, -1},  // ��
    {-1, 0}   // ��
}};

// ����ṹ
struct Position {
  int x, y;

  Position() noexcept : x(0), y(0) {}
  Position(int x, int y) noexcept : x(x), y(y) {}

  bool operator==(Position const& other) const noexcept {
    return x == other.x && y == other.y;
  }

  Position operator+(Direction const& dir) const noexcept {
    return {x + dir.dx, y + dir.dy};
  }
};

// ==================== ����������� ====================
class RandomGenerator {
 private:
  std::random_device rd;
  std::mt19937 gen;

 public:
  RandomGenerator() : gen(rd()) {}

  int getInt(int min, int max) {
    std::uniform_int_distribution<> dis(min, max - 1);
    return dis(gen);
  }

  bool getBool(int probability) {
    return getInt(0, 100) < probability;
  }
};

// ==================== ��Ϸ״̬ ====================
class GameState {
 public:
  int money          = 0;
  int healthPoints   = INITIAL_HEALTH;
  int chestsOpened   = 0;
  int ghostsDefeated = 0;
  std::array<int, ITEM_TYPES> backpack{};

  void reset() noexcept {
    money          = 0;
    healthPoints   = INITIAL_HEALTH;
    chestsOpened   = 0;
    ghostsDefeated = 0;
    backpack.fill(0);
  }

  [[nodiscard]] int calculateBackpackValue() const noexcept {
    int total = 0;
    for (size_t i = 0; i < ITEM_TYPES; ++i) {
      total += backpack[i] * ITEMS[i].value;
    }
    return total;
  }

  [[nodiscard]] int calculateTotalItems() const noexcept {
    return std::accumulate(backpack.begin(), backpack.end(), 0);
  }
};

// ==================== ��ͼ�� ====================
class GameMap {
 private:
  int size_;
  std::vector<std::vector<MapElement>> map_;
  std::vector<std::vector<int>> ghostHP_;
  std::vector<std::vector<bool>> visited_;
  Position playerPos_;
  RandomGenerator& rng_;

 public:
  GameMap(int size, RandomGenerator& rng)
      : size_(size),
        map_(size + 1, std::vector<MapElement>(size + 1, MapElement::Empty)),
        ghostHP_(size + 1, std::vector<int>(size + 1, 0)),
        visited_(size + 1, std::vector<bool>(size + 1, false)),
        playerPos_(2, 2),
        rng_(rng) {}

  void generate() {
    bool pathExists = false;

    while (!pathExists) {
      resetMap();

      // ���ѡ����ʼ�����ɵ�ͼ
      auto startX = rng_.getInt(1, size_ + 1);
      auto startY = rng_.getInt(1, size_ + 1);

      generateRecursive(startX, startY);

      // ����Ƿ���ͨ·
      resetVisited();
      pathExists = checkPath(2, 2);

      if (!pathExists) {
        resetMap();
      }
    }

    // ������Һͳ���
    setElement(2, 2, MapElement::Player);
    setElement(size_ - 1, size_ - 1, MapElement::Door);
    playerPos_ = Position(2, 2);

    // ���ù��
    placeGhosts();

    // ���ñ���
    placeChests();
  }

  [[nodiscard]] bool isInBounds(Position const& pos) const noexcept {
    return pos.x >= 1 && pos.x <= size_ && pos.y >= 1 && pos.y <= size_;
  }

  [[nodiscard]] MapElement getElement(Position const& pos) const {
    return map_[pos.x][pos.y];
  }

  void setElement(int x, int y, MapElement elem) {
    map_[x][y] = elem;
  }

  void setElement(Position const& pos, MapElement elem) {
    map_[pos.x][pos.y] = elem;
  }

  [[nodiscard]] int getGhostHP(Position const& pos) const {
    return ghostHP_[pos.x][pos.y];
  }

  void setGhostHP(Position const& pos, int hp) {
    ghostHP_[pos.x][pos.y] = hp;
  }

  [[nodiscard]] Position getPlayerPos() const noexcept {
    return playerPos_;
  }

  void setPlayerPos(Position const& pos) noexcept {
    playerPos_ = pos;
  }

  [[nodiscard]] int getSize() const noexcept {
    return size_;
  }

  void display() const {
    std::cout << "   ";
    for (int i = 1; i <= size_; ++i) {
      std::cout << std::setw(3) << i;
    }
    std::cout << '\n';

    for (int i = 1; i <= size_; ++i) {
      std::cout << std::setw(2) << i << " ";
      for (int j = 1; j <= size_; ++j) {
        std::cout << "[" << getDisplayChar(Position(i, j)) << "]";
      }
      std::cout << '\n';
    }
  }

 private:
  void resetMap() {
    for (auto& row : map_) {
      std::fill(row.begin(), row.end(), MapElement::Empty);
    }
    for (auto& row : ghostHP_) {
      std::fill(row.begin(), row.end(), 0);
    }
    resetVisited();
  }

  void resetVisited() {
    for (auto& row : visited_) {
      std::fill(row.begin(), row.end(), false);
    }
  }

  void generateRecursive(int x, int y) {
    visited_[x][y] = true;

    // ������ɵ���
    auto value = rng_.getInt(0, 9);
    map_[x][y] = static_cast<MapElement>(value);

    // ���ĸ�������չ
    for (auto const& dir : DIRECTIONS) {
      int nextX = x + dir.dx;
      int nextY = y + dir.dy;

      if (isInBounds(Position(nextX, nextY)) && map_[nextX][nextY] == MapElement::Empty) {
        generateRecursive(nextX, nextY);
      }
    }
  }

  bool checkPath(int x, int y) {
    if (x == size_ - 1 && y == size_ - 1) {
      return true;
    }

    if (static_cast<int>(map_[x][y]) >= static_cast<int>(MapElement::WallStart) && map_[x][y] != MapElement::Door
        && map_[x][y] != MapElement::Ghost && map_[x][y] != MapElement::GhostHurt
        && map_[x][y] != MapElement::GhostWeak) {
      return false;
    }

    visited_[x][y] = true;

    return std::any_of(DIRECTIONS.begin(), DIRECTIONS.end(), [this, x, y](Direction const& dir) {
      int nextX = x + dir.dx;
      int nextY = y + dir.dy;
      return isInBounds(Position(nextX, nextY)) && !visited_[nextX][nextY]
             && (static_cast<int>(map_[nextX][nextY]) < static_cast<int>(MapElement::WallStart)
                 || map_[nextX][nextY] == MapElement::Door || map_[nextX][nextY] == MapElement::Ghost
                 || map_[nextX][nextY] == MapElement::GhostHurt || map_[nextX][nextY] == MapElement::GhostWeak)
             && checkPath(nextX, nextY);
    });
  }

  void placeGhosts() {
    int ghostCount = 3 + rng_.getInt(0, 3);
    for (int i = 0; i < ghostCount; ++i) {
      auto x = rng_.getInt(1, size_ + 1);
      auto y = rng_.getInt(1, size_ + 1);

      if (static_cast<int>(map_[x][y]) < static_cast<int>(MapElement::WallStart)) {
        map_[x][y]     = MapElement::Ghost;
        ghostHP_[x][y] = GHOST_MAX_HP;
      }
    }
  }

  void placeChests() {
    int chestCount = 2 + rng_.getInt(0, 3);
    for (int i = 0; i < chestCount; ++i) {
      auto x = rng_.getInt(1, size_ + 1);
      auto y = rng_.getInt(1, size_ + 1);

      if (static_cast<int>(map_[x][y]) < static_cast<int>(MapElement::WallStart) && (x != 2 || y != 2)
          && (x != size_ - 1 || y != size_ - 1)) {
        map_[x][y] = MapElement::Chest;
      }
    }
  }

  [[nodiscard]] char getDisplayChar(Position const& pos) const {
    auto element = map_[pos.x][pos.y];

    if (element == MapElement::Ghost || element == MapElement::GhostHurt || element == MapElement::GhostWeak) {
      int hp = ghostHP_[pos.x][pos.y];
      if (hp == 3) {
        return 'G';
      }
      if (hp == 2) {
        return 'g';
      }
      if (hp == 1) {
        return '*';
      }
    }

    if (static_cast<int>(element) < static_cast<int>(MapElement::WallStart)) {
      return ' ';
    }

    switch (element) {
      case MapElement::Wall:
      case MapElement::Wall2:
      case MapElement::Wall3:
        return 'W';
      case MapElement::Chest:
        return 'C';
      case MapElement::Player:
        return 'Y';
      case MapElement::Door:
        return 'D';
      default:
        return '?';
    }
  }
};

// ==================== UI������ ====================
class UIManager {
 private:
  std::string lastMessage_;

 public:
  void showMessage(std::string_view msg) {
    lastMessage_ = msg;
  }

  [[nodiscard]] std::string getAndClearMessage() {
    std::string msg = lastMessage_;
    lastMessage_.clear();
    return msg;
  }

  static void displayGameStatus(GameState const& state) {
    std::cout << "==================== ��Ϸ״̬ ====================\n";
    std::cout << "���: " << state.money << "  |  ����ֵ: " << state.healthPoints
              << "  |  �ѿ�����: " << state.chestsOpened << "  |  ���ܹ��: " << state.ghostsDefeated << '\n';
    std::cout << "==================================================\n";
  }

  static void displayBackpack(GameState const& state) {
    Platform::clearScreen();

    std::cout << "�X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[\n";
    std::cout << "�U                    �ҵı���                    �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U  ���: " << std::setw(10) << state.money << " ö                            �U\n";
    std::cout << "�U  ����ֵ: " << state.healthPoints << " / 20                               �U\n";
    std::cout << "�U  �ѿ�������: " << std::setw(3) << state.chestsOpened << " ��                          �U\n";
    std::cout << "�U  ���ܹ��: " << std::setw(3) << state.ghostsDefeated << " ��                            �U\n";

    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U                  ��Ʒ�嵥                      �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";

    bool hasItems = false;
    for (size_t i = 0; i < ITEM_TYPES; ++i) {
      if (state.backpack[i] > 0) {
        hasItems      = true;
        int itemValue = state.backpack[i] * ITEMS[i].value;

        std::cout << "�U  " << ITEMS[i].name << ": ";

        int nameLen = std::string(ITEMS[i].name).length();
        for (int j = 0; j < 12 - nameLen / 3 * 2; ++j) {
          std::cout << " ";
        }

        std::cout << std::setw(3) << state.backpack[i] << "��  ";
        std::cout << "����:" << std::setw(7) << ITEMS[i].value;
        std::cout << "  ��ֵ:" << std::setw(8) << itemValue;
        std::cout << "   �U\n";
      }
    }

    if (!hasItems) {
      std::cout << "�U        �����ǿյģ�ȥѰ�ұ���ɣ�             �U\n";
    }

    int totalItems = state.calculateTotalItems();
    int totalValue = state.calculateBackpackValue();

    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U  ��Ʒ����: " << std::setw(5) << totalItems << " ��                           �U\n";
    std::cout << "�U  ��Ʒ�ܼ�ֵ: " << std::setw(10) << totalValue << " ���                �U\n";
    std::cout << "�^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a\n";
    std::cout << "\n�������������Ϸ...\n";
    Platform::getChar();
  }

  static void displayChestOpening(int totalValue, std::array<int, ITEM_TYPES> const& items) {
    Platform::clearScreen();

    // ���䶯��
    std::cout << "\n\n                    ���ڴ򿪱���...\n";
    std::cout << "                         ___\n";
    std::cout << "                        /   \\\n";
    std::cout << "                       /_____\\\n";
    std::cout << "                      [=======]\n";
    std::cout << "                      [_______]\n";
    Platform::pause(std::chrono::milliseconds(500));

    Platform::clearScreen();
    std::cout << "\n�X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[\n";
    std::cout << "�U                   ��������                     �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U                   �����Ʒ                     �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";

    bool hasLoot = false;
    for (size_t i = 0; i < ITEM_TYPES; ++i) {
      if (items[i] > 0) {
        hasLoot   = true;
        int value = items[i] * ITEMS[i].value;

        std::cout << "�U  " << ITEMS[i].name;

        int nameLen = std::string(ITEMS[i].name).length();
        for (int j = 0; j < 10 - nameLen / 3 * 2; ++j) {
          std::cout << " ";
        }

        std::cout << "x" << std::setw(3) << items[i];
        std::cout << "      ��ֵ: " << std::setw(8) << value << " ���";
        std::cout << "     �U\n";
      }
    }

    if (!hasLoot) {
      std::cout << "�U            �����ǿյģ��治���ˣ�              �U\n";
    }

    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U  �ܼ�ֵ: " << std::setw(10) << totalValue << " ���                       �U\n";
    std::cout << "�^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a\n";

    if (totalValue > 10000) {
      std::cout << "\n                  [�����!]\n";
    } else if (totalValue > 1000) {
      std::cout << "\n                  [�ջ񲻴�!]\n";
    } else if (totalValue > 100) {
      std::cout << "\n                  [������!]\n";
    } else {
      std::cout << "\n                  [��ʤ����]\n";
    }

    std::cout << "\n�����������...\n";
    Platform::getChar();
  }

  static void displayBattleResult(int reward, std::optional<std::string> loot = std::nullopt) {
    Platform::clearScreen();

    std::cout << "\n�X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[\n";
    std::cout << "�U                  ս��ʤ����                    �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U  ��ý��: " << std::setw(8) << reward << " ö                         �U\n";

    if (loot.has_value()) {
      std::cout << "�U  " << std::left << std::setw(46) << loot.value() << "�U\n";
    }

    std::cout << "�^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a\n";
    std::cout << "\n�����������...\n";
    Platform::getChar();
  }

  static void displayHelp() {
    Platform::clearScreen();
    std::cout << "�X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[\n";
    std::cout << "�U                  ���ٰ���                      �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U                  �ƶ�����                      �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U  W - �����ƶ�       A - �����ƶ�               �U\n";
    std::cout << "�U  S - �����ƶ�       D - �����ƶ�               �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U                  ��������                      �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U  O - ����Χ�ı��䣨�Զ�Ѱ�ң�                �U\n";
    std::cout << "�U  K - ������Χ�Ĺ�֣�ÿ��1���˺���             �U\n";
    std::cout << "�U  E - �鿴����                                  �U\n";
    std::cout << "�U  H/? - ��ʾ�˰���                              �U\n";
    std::cout << "�U  R - �˳���Ϸ                                  �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U                  ս��ϵͳ                      �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U  �����3��Ѫ��: [G]=3Ѫ [g]=2Ѫ [*]=1Ѫ        �U\n";
    std::cout << "�U  ÿ�ι������1���˺�                           �U\n";
    std::cout << "�U  �����40%���ʷ��������1-2���˺���            �U\n";
    std::cout << "�U  ���ܹ�ֻ��50-100���                        �U\n";
    std::cout << "�U  30%���ʵ�����Ʒ                               �U\n";
    std::cout << "�^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a\n";
    std::cout << "\n�������������Ϸ...\n";
    Platform::getChar();
  }

  static void displayTutorial() {
    std::cout << "==================== ��Ϸ�̳� ====================\n";
    std::cout << "Ŀ��: ����㵽����ڣ��ռ������еĲƱ���\n";
    std::cout << "\n��ͼ����˵��:\n";
    std::cout << "  [ ] = �յ� (��ͨ��)\n";
    std::cout << "  [W] = ǽ�� (����ͨ��)\n";
    std::cout << "  [C] = ���� (���Դ򿪻�òƱ�)\n";
    std::cout << "  [Y] = ��� (���λ��)\n";
    std::cout << "  [G] = ��� (��Ѫ3��)\n";
    std::cout << "  [g] = ���˹�� (2��Ѫ)\n";
    std::cout << "  [*] = ������� (1��Ѫ)\n";
    std::cout << "  [D] = ���� (Ŀ��ص�)\n";
    std::cout << "\n����˵��:\n";
    std::cout << "  W/A/S/D - ��/��/��/�� �ƶ�\n";
    std::cout << "  O - ����Χ�ı��䣨�Զ�Ѱ�ң�\n";
    std::cout << "  K - ������Χ�Ĺ�֣�ÿ��1���˺���\n";
    std::cout << "  E - �򿪱��� (�鿴��Ʒͳ��)\n";
    std::cout << "  H/? - ��ʾ���ٰ���\n";
    std::cout << "  R - �˳���Ϸ\n";
    std::cout << "\n��Ϸ��ʾ:\n";
    std::cout << "  ? �����3��Ѫ������Ҫ����3�β��ܻ���\n";
    std::cout << "  ? ������˺��м��ʷ���\n";
    std::cout << "  ? ���ܹ�ֿɻ�ý�Һ���Ʒ����\n";
    std::cout << "  ? �ռ�����Ʒ���Զ����뱳��\n";
    std::cout << "  ? ע�Ᵽ���Լ�������ֵ��\n";
    std::cout << "==================================================\n";
    std::cout << "\n���������ʼ��Ϸ...";
    Platform::getChar();
  }
};

// ==================== ��Ϸ���� ====================
class Game {
 private:
  GameState state_;
  std::unique_ptr<GameMap> map_;
  UIManager ui_;
  RandomGenerator rng_;
  int mapSize_;

 public:
  Game() : mapSize_(10) {}

  void run() {
    Platform::setColor();
    displayTitle();

    std::cout << "�������ͼ��С (4-20): ";
    std::cin >> mapSize_;
    mapSize_ = std::clamp(mapSize_, 4, 20);

    state_.reset();
    UIManager::displayTutorial();

    while (playLevel()) {
      // ������һ��
    }

    displayGameOver();
  }

 private:
  bool playLevel() {
    map_ = std::make_unique<GameMap>(mapSize_, rng_);
    map_->generate();

    while (true) {
      Platform::clearScreen();

      // �����Ϸ״̬
      if (state_.healthPoints <= 0) {
        displayDefeat();
        return false;
      }

      // ����Ƿ񵽴��յ�
      if (isAtExit()) {
        if (!displayVictory()) {
          return false;
        }
        break;
      }

      // ��ʾ��Ϸ����
      UIManager::displayGameStatus(state_);
      map_->display();
      std::cout << "==================================================\n";
      std::cout << "���Ѫ��: [G]=3Ѫ  [g]=2Ѫ  [*]=1Ѫ\n";
      std::cout << "����: W/A/S/D�ƶ� | O���� | K���� | E���� | H���� | R�˳�\n";
      std::cout << "��ʾ: �����Ҫ����3�β��ܻ��ܣ�С�ķ�����\n";

      // ��ʾ��Ϣ
      auto msg = ui_.getAndClearMessage();
      if (!msg.empty()) {
        std::cout << ">> " << msg << '\n';
      }

      if (!processInput()) {
        return false;
      }
    }

    return true;
  }

  bool processInput() {
    char input = Platform::getChar();
    input      = std::tolower(input);

    switch (input) {
      case 'w':
      case 'a':
      case 's':
      case 'd':
        handleMovement(input);
        break;
      case 'o':
        openChest();
        break;
      case 'k':
        attackGhost();
        break;
      case 'e':
        UIManager::displayBackpack(state_);
        break;
      case 'h':
      case '?':
        UIManager::displayHelp();
        break;
      case 'r':
        std::cout << "\nȷ��Ҫ�˳���Ϸ��(Y/N): ";
        if (std::tolower(Platform::getChar()) == 'y') {
          return false;
        }
        break;
      default:
        // ������Ч����
        break;
    }

    return true;
  }

  void handleMovement(char direction) {
    auto currentPos    = map_->getPlayerPos();
    Position targetPos = currentPos;

    switch (direction) {
      case 'w':
        targetPos = currentPos + DIRECTIONS[3];
        break;
      case 'a':
        targetPos = currentPos + DIRECTIONS[2];
        break;
      case 's':
        targetPos = currentPos + DIRECTIONS[1];
        break;
      case 'd':
        targetPos = currentPos + DIRECTIONS[0];
        break;
      default:
        ui_.showMessage("��Ч���ƶ�����");
        return;
    }

    if (!map_->isInBounds(targetPos)) {
      ui_.showMessage("�޷��ƶ�����λ�ã�");
      return;
    }

    auto targetElement = map_->getElement(targetPos);

    if (static_cast<int>(targetElement) >= static_cast<int>(MapElement::WallStart)
        && targetElement != MapElement::Door) {
      if (targetElement == MapElement::Ghost || targetElement == MapElement::GhostHurt
          || targetElement == MapElement::GhostWeak) {
        ui_.showMessage("ǰ���й�ֵ�·����K��������");
      } else {
        ui_.showMessage("�޷��ƶ�����λ�ã�");
      }
      return;
    }

    // �ƶ����
    map_->setElement(currentPos, MapElement::Empty);
    map_->setElement(targetPos, MapElement::Player);
    map_->setPlayerPos(targetPos);
  }

  void openChest() {
    auto playerPos = map_->getPlayerPos();
    std::vector<Position> chestPositions;

    // ������Χ�ı���
    for (auto const& dir : DIRECTIONS) {
      auto checkPos = playerPos + dir;
      if (map_->isInBounds(checkPos) && map_->getElement(checkPos) == MapElement::Chest) {
        chestPositions.push_back(checkPos);
      }
    }

    if (chestPositions.empty()) {
      ui_.showMessage("��Χû�б��䣡");
      return;
    }

    // �򿪵�һ������
    auto chestPos = chestPositions[0];
    state_.chestsOpened++;
    map_->setElement(chestPos, MapElement::Empty);

    // ����ս��Ʒ
    std::array<int, ITEM_TYPES> newItems{};
    int totalValue = 0;

    for (size_t i = 0; i < ITEM_TYPES; ++i) {
      int itemCount = (i <= 3) ? rng_.getInt(0, 64) : rng_.getInt(0, 10);
      newItems[i]   = itemCount;

      if (itemCount > 0) {
        state_.backpack[i] += itemCount;
        totalValue         += itemCount * ITEMS[i].value;
      }
    }

    state_.money += totalValue;
    UIManager::displayChestOpening(totalValue, newItems);

    if (chestPositions.size() > 1) {
      std::stringstream msg;
      msg << "��ʾ����Χ���� " << chestPositions.size() - 1 << " �����䣡";
      ui_.showMessage(msg.str());
    }
  }

  void attackGhost() {
    auto playerPos = map_->getPlayerPos();
    std::vector<Position> ghostPositions;

    // ������Χ�Ĺ��
    for (auto const& dir : DIRECTIONS) {
      auto checkPos = playerPos + dir;
      if (map_->isInBounds(checkPos)) {
        auto elem = map_->getElement(checkPos);
        if (elem == MapElement::Ghost || elem == MapElement::GhostHurt || elem == MapElement::GhostWeak) {
          ghostPositions.push_back(checkPos);
        }
      }
    }

    if (ghostPositions.empty()) {
      ui_.showMessage("��Χû�й�֣�");
      return;
    }

    // ������һ�����
    auto ghostPos  = ghostPositions[0];
    int currentHP  = map_->getGhostHP(ghostPos);
    currentHP     -= PLAYER_DAMAGE;
    map_->setGhostHP(ghostPos, currentHP);

    if (currentHP <= 0) {
      // ��ֱ�����
      map_->setElement(ghostPos, MapElement::Empty);
      state_.ghostsDefeated++;

      int reward    = 50 + rng_.getInt(0, 51);
      state_.money += reward;

      std::optional<std::string> loot;

      // ������Ʒ
      if (rng_.getBool(30)) {
        int itemType               = rng_.getInt(0, 4);
        int itemCount              = 1 + rng_.getInt(0, 3);
        state_.backpack[itemType] += itemCount;

        std::stringstream ss;
        ss << "������Ʒ: " << ITEMS[itemType].name << " x" << itemCount;
        loot = ss.str();
      }

      UIManager::displayBattleResult(reward, loot);

      if (ghostPositions.size() > 1) {
        std::stringstream warning;
        warning << "���棺��Χ���� " << ghostPositions.size() - 1 << " ����֣�";
        ui_.showMessage(warning.str());
      }
    } else {
      // �������
      std::stringstream msg;
      msg << "������֣�ʣ��Ѫ��:" << currentHP << "/" << GHOST_MAX_HP;

      // ���¹��״̬
      if (currentHP == 2) {
        map_->setElement(ghostPos, MapElement::GhostHurt);
      } else if (currentHP == 1) {
        map_->setElement(ghostPos, MapElement::GhostWeak);
      }

      // ��ַ���
      if (rng_.getBool(40)) {
        int damage           = 1 + rng_.getInt(0, 2);
        state_.healthPoints -= damage;
        msg << " ����-" << damage << "HP!";

        if (state_.healthPoints <= 0) {
          ui_.showMessage("�㱻��ֻ����ˣ���Ϸ������");
          return;
        }
      }

      if (ghostPositions.size() > 1) {
        msg << " [����" << ghostPositions.size() - 1 << "�����]";
      }

      ui_.showMessage(msg.str());
    }
  }

  [[nodiscard]] bool isAtExit() const {
    auto pos = map_->getPlayerPos();
    return pos.x == mapSize_ - 1 && pos.y == mapSize_ - 1;
  }

  bool displayVictory() {
    std::cout << "�X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[\n";
    std::cout << "�U                  ��ϲͨ�أ�                    �U\n";
    std::cout << "�U              ��ɹ������˳��ڣ�                �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";

    int levelBonus       = 100 + (mapSize_ * 10);
    state_.money        += levelBonus;
    state_.healthPoints  = std::min(state_.healthPoints + 2, INITIAL_HEALTH);

    std::cout << "�U  ͨ�ؽ���: " << std::setw(8) << levelBonus << " ���                       �U\n";
    std::cout << "�U  ����ֵ�ָ�2�㣡��ǰ����: " << std::setw(2) << state_.healthPoints << "/20                �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U                  ��ǰͳ��                      �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U  ���: " << std::setw(10) << state_.money << " ö                           �U\n";
    std::cout << "�U  �ѿ�����: " << std::setw(8) << state_.chestsOpened << " ��                         �U\n";
    std::cout << "�U  ���ܹ��: " << std::setw(8) << state_.ghostsDefeated << " ��                         �U\n";
    std::cout << "�^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a\n";

    std::cout << "\nѡ��: [0] �˳���Ϸ  [1] ��һ��: ";

    int choice;
    std::cin >> choice;

    return choice != 0;
  }

  void displayDefeat() const {
    std::cout << "�X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[\n";
    std::cout << "�U                  ��Ϸ������                    �U\n";
    std::cout << "�U              �㱻��ֻ����ˣ�                  �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U                  ����ͳ��                      �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U  �ܽ��: " << std::setw(10) << state_.money << " ö                         �U\n";
    std::cout << "�U  ��������: " << std::setw(8) << state_.chestsOpened << " ��                         �U\n";
    std::cout << "�U  ���ܹ��: " << std::setw(8) << state_.ghostsDefeated << " ��                         �U\n";
    std::cout << "�^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a\n";
    Platform::pause(std::chrono::milliseconds(5000));
  }

  void displayGameOver() const {
    Platform::clearScreen();
    std::cout << "�X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[\n";
    std::cout << "�U                  ��Ϸ������                    �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U                  ����ս��                      �U\n";
    std::cout << "�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g\n";
    std::cout << "�U  �ܽ��: " << std::setw(10) << state_.money << " ö                         �U\n";
    std::cout << "�U  ��������: " << std::setw(8) << state_.chestsOpened << " ��                         �U\n";
    std::cout << "�U  ���ܹ��: " << std::setw(8) << state_.ghostsDefeated << " ��                         �U\n";
    std::cout << "�^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a\n";
    std::cout << "\n           ��л���棡�´��ټ���\n";
    Platform::pause(std::chrono::milliseconds(2000));
  }

  static void displayTitle() {
    std::cout << "\n\n\n\n\n";
    std::cout << "                    �X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[\n";
    std::cout << "                    �U     CHESTS IN MAPS     �U\n";
    std::cout << "                    �U      ����̽����Ϸ      �U\n";
    std::cout << "                    �U     CANARY EDITION     �U\n";
    std::cout << "                    �^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a\n";
    std::cout << "\n\n\n\n\n";
    std::cout << "                       ���������ʼ��Ϸ...\n";
    std::cout << "\n\n\n\n\n\n\n";
    std::cout << "Version: Canary 4.1\n";

    Platform::getChar();
    Platform::clearScreen();
  }
};
}  // namespace ChestsInMaps

// ==================== ������ ====================
int main() {
  try {
    ChestsInMaps::Game game;
    game.run();
  } catch (std::exception const& e) {
    std::cerr << "��Ϸ��������: " << e.what() << '\n';
    return 1;
  }

  return 0;
}