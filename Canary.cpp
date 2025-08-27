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
#include <windows.h>  // 添加Windows头文件

// Windows平台函数
namespace Platform {
inline void clearScreen() noexcept {
  system("cls");
}

inline void setColor() noexcept {
  system("color 03");
}

// 初始化控制台UTF-8支持
inline void initConsole() noexcept {
  SetConsoleOutputCP(CP_UTF8);  // 设置UTF-8输出
  SetConsoleCP(CP_UTF8);         // 设置UTF-8输入
}

inline void pause(std::chrono::milliseconds ms) noexcept {
  std::this_thread::sleep_for(ms);
}

inline char getChar() noexcept {
  return static_cast<char>(getch());
}
}  // namespace Platform

// 游戏命名空间
namespace ChestsInMaps {

// ==================== 常量定义 ====================
inline constexpr int MAX_MAP_SIZE    = 55;
inline constexpr int MOVE_DIRECTIONS = 4;
inline constexpr int GHOST_MAX_HP    = 3;
inline constexpr int PLAYER_DAMAGE   = 1;
inline constexpr int INITIAL_HEALTH  = 20;
inline constexpr int ITEM_TYPES      = 8;

// 强类型枚举 - 地图元素
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

// 物品系统
struct Item {
  std::string_view name;
  int value;
};

// 物品定义
inline constexpr std::array<Item, ITEM_TYPES> ITEMS = {{{"原石", 1},
                                                        {"煤炭", 5},
                                                        {"铁锭", 100},
                                                        {"红石", 150},
                                                        {"青金石", 1000},
                                                        {"绿宝石", 1000},
                                                        {"金锭", 100000},
                                                        {"钻石", 1000000}}};

// 方向向量
struct Direction {
  int dx, dy;
};

inline constexpr std::array<Direction, MOVE_DIRECTIONS> DIRECTIONS = {{
    {0, 1},   // 右
    {1, 0},   // 下
    {0, -1},  // 左
    {-1, 0}   // 上
}};

// 坐标结构
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

// ==================== 随机数生成器 ====================
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

// ==================== 游戏状态 ====================
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

// ==================== 地图类 ====================
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

      // 随机选择起始点生成地图
      auto startX = rng_.getInt(1, size_ + 1);
      auto startY = rng_.getInt(1, size_ + 1);

      generateRecursive(startX, startY);

      // 检查是否有通路
      resetVisited();
      pathExists = checkPath(2, 2);

      if (!pathExists) {
        resetMap();
      }
    }

    // 设置玩家和出口
    setElement(2, 2, MapElement::Player);
    setElement(size_ - 1, size_ - 1, MapElement::Door);
    playerPos_ = Position(2, 2);

    // 放置鬼怪
    placeGhosts();

    // 放置宝箱
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

    // 随机生成地形
    auto value = rng_.getInt(0, 9);
    map_[x][y] = static_cast<MapElement>(value);

    // 向四个方向扩展
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

    return std::any_of(DIRECTIONS.begin(), DIRECTIONS.end(), [&](Direction const& dir) {
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

// ==================== UI管理器 ====================
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
    std::cout << "==================== 游戏状态 ====================\n";
    std::cout << "金币: " << state.money << "  |  生命值: " << state.healthPoints
              << "  |  已开宝箱: " << state.chestsOpened << "  |  击败鬼怪: " << state.ghostsDefeated << '\n';
    std::cout << "==================================================\n";
  }

  static void displayBackpack(GameState const& state) {
    Platform::clearScreen();

    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║                    我的背包                    ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║  金币: " << std::setw(10) << state.money << " 枚                            ║\n";
    std::cout << "║  生命值: " << state.healthPoints << " / 20                               ║\n";
    std::cout << "║  已开启宝箱: " << std::setw(3) << state.chestsOpened << " 个                          ║\n";
    std::cout << "║  击败鬼怪: " << std::setw(3) << state.ghostsDefeated << " 个                            ║\n";

    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║                  物品清单                      ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";

    // 使用 std::any_of 检查是否有物品
    bool hasItems = std::any_of(state.backpack.begin(), state.backpack.end(), [](int count) { return count > 0; });

    for (size_t i = 0; i < ITEM_TYPES; ++i) {
      if (state.backpack[i] > 0) {
        int itemValue = state.backpack[i] * ITEMS[i].value;

        std::cout << "║  " << ITEMS[i].name << ": ";

        int nameLen = std::string(ITEMS[i].name).length();
        for (int j = 0; j < 12 - nameLen / 3 * 2; ++j) {
          std::cout << " ";
        }

        std::cout << std::setw(3) << state.backpack[i] << "个  ";
        std::cout << "单价:" << std::setw(7) << ITEMS[i].value;
        std::cout << "  总值:" << std::setw(8) << itemValue;
        std::cout << "   ║\n";
      }
    }

    if (!hasItems) {
      std::cout << "║        背包是空的，去寻找宝箱吧！             ║\n";
    }

    int totalItems = state.calculateTotalItems();
    int totalValue = state.calculateBackpackValue();

    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║  物品总数: " << std::setw(5) << totalItems << " 件                           ║\n";
    std::cout << "║  物品总价值: " << std::setw(10) << totalValue << " 金币                ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    std::cout << "\n按任意键返回游戏...\n";
    Platform::getChar();
  }

  static void displayChestOpening(int totalValue, std::array<int, ITEM_TYPES> const& items) {
    Platform::clearScreen();

    // 开箱动画
    std::cout << "\n\n                    正在打开宝箱...\n";
    std::cout << "                         ___\n";
    std::cout << "                        /   \\\n";
    std::cout << "                       /_____\\\n";
    std::cout << "                      [=======]\n";
    std::cout << "                      [_______]\n";
    Platform::pause(std::chrono::milliseconds(500));

    Platform::clearScreen();
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║                   开启宝箱                     ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║                   获得物品                     ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";

    // 使用 std::any_of 检查是否有战利品
    bool hasLoot = std::any_of(items.begin(), items.end(), [](int count) { return count > 0; });

    for (size_t i = 0; i < ITEM_TYPES; ++i) {
      if (items[i] > 0) {
        int value = items[i] * ITEMS[i].value;

        std::cout << "║  " << ITEMS[i].name;

        int nameLen = std::string(ITEMS[i].name).length();
        for (int j = 0; j < 10 - nameLen / 3 * 2; ++j) {
          std::cout << " ";
        }

        std::cout << "x" << std::setw(3) << items[i];
        std::cout << "      价值: " << std::setw(8) << value << " 金币";
        std::cout << "    ║\n";
      }
    }

    if (!hasLoot) {
      std::cout << "║            宝箱是空的，真不走运！              ║\n";
    }

    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║  总价值: " << std::setw(10) << totalValue << " 金币                           ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";

    if (totalValue > 10000) {
      std::cout << "\n                  [大丰收!]\n";
    } else if (totalValue > 1000) {
      std::cout << "\n                  [收获不错!]\n";
    } else if (totalValue > 100) {
      std::cout << "\n                  [还可以!]\n";
    } else {
      std::cout << "\n                  [聊胜于无]\n";
    }

    std::cout << "\n按任意键继续...\n";
    Platform::getChar();
  }

  static void displayBattleResult(int reward, std::optional<std::string> loot = std::nullopt) {
    Platform::clearScreen();

    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║                  战斗胜利！                    ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║  获得金币: " << std::setw(8) << reward << " 枚                        ║\n";

    if (loot.has_value()) {
      std::cout << "║  " << std::left << std::setw(46) << loot.value() << "║\n";
    }

    std::cout << "╚════════════════════════════════════════════════╝\n";
    std::cout << "\n按任意键继续...\n";
    Platform::getChar();
  }

  static void displayHelp() {
    Platform::clearScreen();
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║                  快速帮助                      ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║                  移动操作                      ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║  W - 向上移动       A - 向左移动              ║\n";
    std::cout << "║  S - 向下移动       D - 向右移动              ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║                  动作操作                      ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║  O - 打开周围的宝箱（自动寻找）               ║\n";
    std::cout << "║  K - 攻击周围的鬼怪（每次1点伤害）            ║\n";
    std::cout << "║  E - 查看背包                                  ║\n";
    std::cout << "║  H/? - 显示此帮助                             ║\n";
    std::cout << "║  R - 退出游戏                                  ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║                  战斗系统                      ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║  鬼怪有3点血量: [G]=3血 [g]=2血 [*]=1血       ║\n";
    std::cout << "║  每次攻击造成1点伤害                          ║\n";
    std::cout << "║  鬼怪有40%概率反击（造成1-2点伤害）           ║\n";
    std::cout << "║  击败鬼怪获得50-100金币                       ║\n";
    std::cout << "║  30%概率掉落物品                              ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    std::cout << "\n按任意键返回游戏...\n";
    Platform::getChar();
  }

  static void displayTutorial() {
    std::cout << "==================== 游戏教程 ====================\n";
    std::cout << "目标: 从起点到达出口，收集宝箱中的财宝！\n";
    std::cout << "\n地图符号说明:\n";
    std::cout << "  [ ] = 空地 (可通行)\n";
    std::cout << "  [W] = 墙壁 (不可通行)\n";
    std::cout << "  [C] = 宝箱 (可以打开获得财宝)\n";
    std::cout << "  [Y] = 玩家 (你的位置)\n";
    std::cout << "  [G] = 鬼怪 (满血3点)\n";
    std::cout << "  [g] = 受伤鬼怪 (2点血)\n";
    std::cout << "  [*] = 虚弱鬼怪 (1点血)\n";
    std::cout << "  [D] = 出口 (目标地点)\n";
    std::cout << "\n操作说明:\n";
    std::cout << "  W/A/S/D - 上/左/下/右 移动\n";
    std::cout << "  O - 打开周围的宝箱（自动寻找）\n";
    std::cout << "  K - 攻击周围的鬼怪（每次1点伤害）\n";
    std::cout << "  E - 打开背包 (查看物品统计)\n";
    std::cout << "  H/? - 显示快速帮助\n";
    std::cout << "  R - 退出游戏\n";
    std::cout << "\n游戏提示:\n";
    std::cout << "  • 鬼怪有3点血量，需要攻击3次才能击败\n";
    std::cout << "  • 鬼怪受伤后有几率反击\n";
    std::cout << "  • 击败鬼怪可获得金币和物品奖励\n";
    std::cout << "  • 收集的物品会自动存入背包\n";
    std::cout << "  • 注意保护自己的生命值！\n";
    std::cout << "==================================================\n";
    std::cout << "\n按任意键开始游戏...";
    Platform::getChar();
  }
};

// ==================== 游戏引擎 ====================
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
    Platform::initConsole();  // 初始化控制台UTF-8
    Platform::setColor();
    displayTitle();

    std::cout << "请输入地图大小 (4-20): ";
    std::cin >> mapSize_;
    mapSize_ = std::clamp(mapSize_, 4, 20);

    state_.reset();
    UIManager::displayTutorial();

    while (playLevel()) {
      // 继续下一关
    }

    displayGameOver();
  }

 private:
  bool playLevel() {
    map_ = std::make_unique<GameMap>(mapSize_, rng_);
    map_->generate();

    while (true) {
      Platform::clearScreen();

      // 检查游戏状态
      if (state_.healthPoints <= 0) {
        displayDefeat();
        return false;
      }

      // 检查是否到达终点
      if (isAtExit()) {
        if (!displayVictory()) {
          return false;
        }
        break;
      }

      // 显示游戏界面
      UIManager::displayGameStatus(state_);
      map_->display();
      std::cout << "==================================================\n";
      std::cout << "鬼怪血量: [G]=3血  [g]=2血  [*]=1血\n";
      std::cout << "操作: W/A/S/D移动 | O开箱 | K攻击 | E背包 | H帮助 | R退出\n";
      std::cout << "提示: 鬼怪需要攻击3次才能击败，小心反击！\n";

      // 显示消息
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
        std::cout << "\n确定要退出游戏吗？(Y/N): ";
        if (std::tolower(Platform::getChar()) == 'y') {
          return false;
        }
        break;
      default:
        // 忽略无效输入
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
        ui_.showMessage("无效的移动方向！");
        return;
    }

    if (!map_->isInBounds(targetPos)) {
      ui_.showMessage("无法移动到该位置！");
      return;
    }

    auto targetElement = map_->getElement(targetPos);

    if (static_cast<int>(targetElement) >= static_cast<int>(MapElement::WallStart)
        && targetElement != MapElement::Door) {
      if (targetElement == MapElement::Ghost || targetElement == MapElement::GhostHurt
          || targetElement == MapElement::GhostWeak) {
        ui_.showMessage("前方有鬼怪挡路！按K攻击它！");
      } else {
        ui_.showMessage("无法移动到该位置！");
      }
      return;
    }

    // 移动玩家
    map_->setElement(currentPos, MapElement::Empty);
    map_->setElement(targetPos, MapElement::Player);
    map_->setPlayerPos(targetPos);
  }

  void openChest() {
    auto playerPos = map_->getPlayerPos();
    std::vector<Position> chestPositions;

    // 查找周围的宝箱
    for (auto const& dir : DIRECTIONS) {
      auto checkPos = playerPos + dir;
      if (map_->isInBounds(checkPos) && map_->getElement(checkPos) == MapElement::Chest) {
        chestPositions.push_back(checkPos);
      }
    }

    if (chestPositions.empty()) {
      ui_.showMessage("周围没有宝箱！");
      return;
    }

    // 打开第一个宝箱
    auto chestPos = chestPositions[0];
    state_.chestsOpened++;
    map_->setElement(chestPos, MapElement::Empty);

    // 生成战利品
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
      msg << "提示：周围还有 " << chestPositions.size() - 1 << " 个宝箱！";
      ui_.showMessage(msg.str());
    }
  }

  void attackGhost() {
    auto playerPos = map_->getPlayerPos();
    std::vector<Position> ghostPositions;

    // 查找周围的鬼怪
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
      ui_.showMessage("周围没有鬼怪！");
      return;
    }

    // 攻击第一个鬼怪
    auto ghostPos  = ghostPositions[0];
    int currentHP  = map_->getGhostHP(ghostPos);
    currentHP     -= PLAYER_DAMAGE;
    map_->setGhostHP(ghostPos, currentHP);

    if (currentHP <= 0) {
      // 鬼怪被击败
      map_->setElement(ghostPos, MapElement::Empty);
      state_.ghostsDefeated++;

      int reward    = 50 + rng_.getInt(0, 51);
      state_.money += reward;

      std::optional<std::string> loot;

      // 掉落物品
      if (rng_.getBool(30)) {
        int itemType               = rng_.getInt(0, 4);
        int itemCount              = 1 + rng_.getInt(0, 3);
        state_.backpack[itemType] += itemCount;

        std::stringstream ss;
        ss << "掉落物品: " << ITEMS[itemType].name << " x" << itemCount;
        loot = ss.str();
      }

      UIManager::displayBattleResult(reward, loot);

      if (ghostPositions.size() > 1) {
        std::stringstream warning;
        warning << "警告：周围还有 " << ghostPositions.size() - 1 << " 个鬼怪！";
        ui_.showMessage(warning.str());
      }
    } else {
      // 鬼怪受伤
      std::stringstream msg;
      msg << "攻击鬼怪！剩余血量:" << currentHP << "/" << GHOST_MAX_HP;

      // 更新鬼怪状态
      if (currentHP == 2) {
        map_->setElement(ghostPos, MapElement::GhostHurt);
      } else if (currentHP == 1) {
        map_->setElement(ghostPos, MapElement::GhostWeak);
      }

      // 鬼怪反击
      if (rng_.getBool(40)) {
        int damage           = 1 + rng_.getInt(0, 2);
        state_.healthPoints -= damage;
        msg << " 反击-" << damage << "HP!";

        if (state_.healthPoints <= 0) {
          ui_.showMessage("你被鬼怪击败了！游戏结束！");
          return;
        }
      }

      if (ghostPositions.size() > 1) {
        msg << " [还有" << ghostPositions.size() - 1 << "个鬼怪]";
      }

      ui_.showMessage(msg.str());
    }
  }

  [[nodiscard]] bool isAtExit() const {
    auto pos = map_->getPlayerPos();
    return pos.x == mapSize_ - 1 && pos.y == mapSize_ - 1;
  }

  bool displayVictory() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║                  恭喜通关！                    ║\n";
    std::cout << "║              你成功到达了出口！                ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";

    int levelBonus       = 100 + (mapSize_ * 10);
    state_.money        += levelBonus;
    state_.healthPoints  = std::min(state_.healthPoints + 2, INITIAL_HEALTH);

    std::cout << "║  通关奖励: " << std::setw(8) << levelBonus << " 金币                      ║\n";
    std::cout << "║  生命值恢复2点！当前生命: " << std::setw(2) << state_.healthPoints << "/20            ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║                  当前统计                      ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║  金币: " << std::setw(10) << state_.money << " 枚                           ║\n";
    std::cout << "║  已开宝箱: " << std::setw(8) << state_.chestsOpened << " 个                         ║\n";
    std::cout << "║  击败鬼怪: " << std::setw(8) << state_.ghostsDefeated << " 个                         ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";

    std::cout << "\n选择: [0] 退出游戏  [1] 下一关: ";

    int choice;
    std::cin >> choice;

    return choice != 0;
  }

  void displayDefeat() const {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║                  游戏结束！                    ║\n";
    std::cout << "║              你被鬼怪击败了！                  ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║                  最终统计                      ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║  总金币: " << std::setw(10) << state_.money << " 枚                         ║\n";
    std::cout << "║  开启宝箱: " << std::setw(8) << state_.chestsOpened << " 个                         ║\n";
    std::cout << "║  击败鬼怪: " << std::setw(8) << state_.ghostsDefeated << " 个                         ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    Platform::pause(std::chrono::milliseconds(5000));
  }

  void displayGameOver() const {
    Platform::clearScreen();
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║                  游戏结束！                    ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║                  最终战绩                      ║\n";
    std::cout << "╠════════════════════════════════════════════════╣\n";
    std::cout << "║  总金币: " << std::setw(10) << state_.money << " 枚                         ║\n";
    std::cout << "║  开启宝箱: " << std::setw(8) << state_.chestsOpened << " 个                         ║\n";
    std::cout << "║  击败鬼怪: " << std::setw(8) << state_.ghostsDefeated << " 个                         ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    std::cout << "\n           感谢游玩！下次再见！\n";
    Platform::pause(std::chrono::milliseconds(2000));
  }

  static void displayTitle() {
    std::cout << "\n\n\n\n\n";
    std::cout << "                    ╔════════════════════════╗\n";
    std::cout << "                    ║     CHESTS IN MAPS     ║\n";
    std::cout << "                    ║      宝箱探险游戏      ║\n";
    std::cout << "                    ║   C++ CANARY EDITION   ║\n";
    std::cout << "                    ╚════════════════════════╝\n";
    std::cout << "\n\n\n\n\n";
    std::cout << "                       按任意键开始游戏...\n";
    std::cout << "\n\n\n\n\n\n\n";
    std::cout << "Version: C++ Canary Edition 4.1 Fixed\n";

    Platform::getChar();
    Platform::clearScreen();
  }
};
}  // namespace ChestsInMaps

// ==================== 主函数 ====================
int main() {
  try {
    ChestsInMaps::Game game;
    game.run();
  } catch (std::exception const& e) {
    std::cerr << "游戏发生错误: " << e.what() << '\n';
    return 1;
  }

  return 0;
}