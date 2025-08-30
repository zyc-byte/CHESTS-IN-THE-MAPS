#include <algorithm>
#include <array>
#include <cctype>
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
#include <windows.h>

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
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
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
inline constexpr int MAX_MAP_SIZE     = 55;
inline constexpr int MOVE_DIRECTIONS  = 4;
inline constexpr int GHOST_MAX_HP     = 3;
inline constexpr int SKELETON_MAX_HP  = 3;
inline constexpr int PLAYER_DAMAGE    = 1;
inline constexpr int INITIAL_HEALTH   = 20;
inline constexpr int INITIAL_FULLNESS = 20;
inline constexpr int ITEM_TYPES       = 14;
inline constexpr int MAX_ARROWS       = 200;
inline constexpr int MAX_ENEMIES      = 5;

// 强类型枚举 - 地图元素
enum class MapElement : std::uint8_t {
  Empty        = 0,
  WallStart    = 5,
  Wall         = 5,
  Wall2        = 6,
  Wall3        = 7,
  Chest        = 8,
  Ghost        = 9,
  Player       = 10,
  Door         = 11,
  GhostHurt    = 12,
  GhostWeak    = 13,
  Skeleton     = 14,
  SkeletonHurt = 15,
  SkeletonWeak = 16,
  Arrow        = 17
};

// 物品系统
struct Item {
  std::string_view name;
  int value;
  bool sellable;
};

// 物品定义 - 扩展为14种物品
inline constexpr std::array<Item, ITEM_TYPES> ITEMS = {{{"原石", 1, true},
                                                        {"煤炭", 5, true},
                                                        {"铁锭", 100, true},
                                                        {"金锭", 100, true},
                                                        {"红石", 200, true},
                                                        {"青金石", 300, true},
                                                        {"绿宝石", 1000, true},
                                                        {"钻石", 5000, true},
                                                        {"末影珍珠", 100000, false},
                                                        {"治疗药水", 500000, false},
                                                        {"喷溅型伤害药水", 300000, false},
                                                        {"腐肉", 0, false},
                                                        {"骨头", 0, false},
                                                        {"面包", 10, false}}};

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

  bool operator!=(Position const& other) const noexcept {
    return !(*this == other);
  }

  Position operator+(Direction const& dir) const noexcept {
    return {x + dir.dx, y + dir.dy};
  }

  [[nodiscard]] int distanceTo(Position const& other) const noexcept {
    return std::abs(x - other.x) + std::abs(y - other.y);
  }
};

// 箭矢结构
struct Arrow {
  Position pos;
  int direction;  // 0=右, 1=下, 2=左, 3=上
  bool active;

  Arrow() noexcept : pos(0, 0), direction(0), active(false) {}
};

// 敌人结构
struct Enemy {
  Position pos;
  int hp;
  MapElement type;
  bool active;

  Enemy() noexcept : pos(0, 0), hp(0), type(MapElement::Ghost), active(false) {}
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
  int money             = 0;
  int healthPoints      = INITIAL_HEALTH;
  int fullness          = INITIAL_FULLNESS;
  int chestsOpened      = 0;
  int ghostsDefeated    = 0;
  int skeletonsDefeated = 0;
  int steps             = 0;
  std::array<int, ITEM_TYPES> backpack{};

  void reset() noexcept {
    money             = 0;
    healthPoints      = INITIAL_HEALTH;
    fullness          = INITIAL_FULLNESS;
    chestsOpened      = 0;
    ghostsDefeated    = 0;
    skeletonsDefeated = 0;
    steps             = 0;
    backpack.fill(0);
  }

  [[nodiscard]] int calculateBackpackValue() const noexcept {
    int total = 0;
    for (size_t i = 0; i < 8; ++i) {  // 只计算前8种可售物品
      if (ITEMS[i].sellable) {
        total += backpack[i] * ITEMS[i].value;
      }
    }
    return total;
  }

  [[nodiscard]] int calculateTotalItems() const noexcept {
    return std::accumulate(backpack.begin(), backpack.end(), 0);
  }

  void updateFullness() noexcept {
    steps++;
    if (steps > 0 && steps % 10 == 0 && fullness > 0) {
      fullness--;
    }
  }

  void updateHealth() noexcept {
    if (fullness == 20) {
      healthPoints = std::min(healthPoints + 1, INITIAL_HEALTH);
    } else if (fullness == 0) {
      healthPoints--;
    }
  }
};

// ==================== UTF-8 宽度与边框工具 ====================
namespace Ui {

  // 所有盒子内侧可见宽度（不含左右边框字符）
  inline constexpr int INNER_WIDTH = 48;

  // 判断常见宽字符：CJK 与全角
  inline int charDisplayWidth(char32_t cp) {
    if ((cp >= 0x4E'00 && cp <= 0x9F'FF) ||      // CJK
        (cp >= 0x34'00 && cp <= 0x4D'BF) ||      // CJK Ext-A
        (cp >= 0xF9'00 && cp <= 0xFA'FF) ||      // CJK 兼容表意
        (cp >= 0x2'00'00 && cp <= 0x2'FF'FF) ||  // 扩展块（保守按2）
        (cp >= 0x30'00 && cp <= 0x30'3F) ||      // CJK 符号和标点
        (cp >= 0xFF'01 && cp <= 0xFF'60) ||      // 全角 ASCII 变体
        (cp >= 0xFF'E0 && cp <= 0xFF'E6)) {      // 全角符号
      return 2;
    }
    return 1;
  }

  // 简单 UTF-8 -> UTF-32 解码（足够稳定）
  inline std::u32string utf8ToUtf32(std::string_view s) {
    std::u32string out;
    out.reserve(s.size());
    unsigned char const* p   = reinterpret_cast<unsigned char const*>(s.data());
    unsigned char const* end = p + s.size();
    while (p < end) {
      uint32_t cp;
      if (*p < 0x80) {
        cp = *p++;
      } else if ((*p >> 5) == 0x6 && p + 1 < end) {
        cp  = ((*p & 0x1F) << 6) | (p[1] & 0x3F);
        p  += 2;
      } else if ((*p >> 4) == 0xE && p + 2 < end) {
        cp  = ((*p & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
        p  += 3;
      } else if ((*p >> 3) == 0x1E && p + 3 < end) {
        cp  = ((*p & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
        p  += 4;
      } else {
        ++p;
        continue;
      }
      out.push_back(static_cast<char32_t>(cp));
    }
    return out;
  }

  inline int displayWidth(std::string_view s) {
    int w    = 0;
    auto u32 = utf8ToUtf32(s);
    for (auto cp : u32) {
      w += charDisplayWidth(cp);
    }
    return w;
  }

  inline std::string padRight(std::string_view s, int total_display_width) {
    int w      = displayWidth(s);
    int spaces = std::max(0, total_display_width - w);
    return std::string(s) + std::string(spaces, ' ');
  }

  inline void printTop() {
    std::cout << "╔";
    for (int i = 0; i < INNER_WIDTH; ++i) {
      std::cout << "═";
    }
    std::cout << "╗\n";
  }

  inline void printMid() {
    std::cout << "╠";
    for (int i = 0; i < INNER_WIDTH; ++i) {
      std::cout << "═";
    }
    std::cout << "╣\n";
  }

  inline void printBottom() {
    std::cout << "╚";
    for (int i = 0; i < INNER_WIDTH; ++i) {
      std::cout << "═";
    }
    std::cout << "╝\n";
  }

  inline void printLine(std::string_view content) {
    std::string left   = " " + std::string(content);
    std::string filled = padRight(left, INNER_WIDTH - 1);
    if (displayWidth(filled) < INNER_WIDTH - 1) {
      filled += std::string((INNER_WIDTH - 1) - displayWidth(filled), ' ');
    }
    std::cout << "║" << filled << " ║\n";
  }

  inline void printKV(std::string_view left, std::string_view right) {
    std::string prefix = "  " + std::string(left) + ": ";
    int used           = displayWidth(prefix) + displayWidth(right) + 1;
    int pad            = std::max(0, INNER_WIDTH - used);
    std::cout << "║" << prefix << std::string(pad, ' ') << right << " ║\n";
  }

  inline void printTitleBox(std::string_view title, std::initializer_list<std::string> lines = {}) {
    printTop();
    printLine(title);
    if (!lines.size()) {
      printBottom();
      return;
    }
    printMid();
    for (auto& l : lines) {
      printLine(l);
    }
    printBottom();
  }

  // 新增：格式化列表项函数
  inline void printListItem(std::string_view item, std::string_view description = "") {
    if (description.empty()) {
      printLine(" • " + std::string(item));
    } else {
      // 格式化列表项和描述
      std::string formatted = " • " + std::string(item);
      int itemWidth         = displayWidth(formatted);

      // 根据项目长度决定对齐位置
      int targetWidth = 24;  // 对齐到第24列
      int spaces      = std::max(1, targetWidth - itemWidth);

      // 如果描述太长，需要确保不超出边界
      std::string desc = std::string(description);
      int totalWidth   = itemWidth + spaces + displayWidth(desc);

      if (totalWidth > INNER_WIDTH - 2) {
        // 如果太长，减少空格或截断描述
        spaces = 2;  // 最少保留2个空格
      }

      formatted += std::string(spaces, ' ') + desc;
      printLine(formatted);
    }
  }

  // 新增：带前缀符号的列表项函数
  inline void printSymbolItem(std::string_view symbol, std::string_view name, std::string_view description = "") {
    std::string item = "[" + std::string(symbol) + "] " + std::string(name);
    if (description.empty()) {
      printLine(" • " + item);
    } else {
      std::string formatted = " • " + item;
      int itemWidth         = displayWidth(formatted);
      int targetWidth       = 20;  // 符号项对齐位置
      int spaces            = std::max(1, targetWidth - itemWidth);

      formatted += std::string(spaces, ' ') + std::string(description);
      printLine(formatted);
    }
  }

}  // namespace Ui

// ==================== 地图类 ====================
class GameMap {
 private:
  int size_;
  std::vector<std::vector<MapElement>> map_;
  std::vector<std::vector<int>> enemyHP_;
  std::vector<std::vector<bool>> visited_;
  std::vector<Enemy> ghosts_;
  std::vector<Enemy> skeletons_;
  std::vector<Arrow> arrows_;
  Position playerPos_;
  RandomGenerator& rng_;
  int arrowCount_ = 0;

 public:
  GameMap(int size, RandomGenerator& rng)
      : size_(size),
        map_(size + 1, std::vector<MapElement>(size + 1, MapElement::Empty)),
        enemyHP_(size + 1, std::vector<int>(size + 1, 0)),
        visited_(size + 1, std::vector<bool>(size + 1, false)),
        ghosts_(MAX_ENEMIES),
        skeletons_(MAX_ENEMIES),
        arrows_(MAX_ARROWS),
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

    // 放置敌人
    placeGhosts();
    placeSkeletons();

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

  [[nodiscard]] int getEnemyHP(Position const& pos) const {
    return enemyHP_[pos.x][pos.y];
  }

  void setEnemyHP(Position const& pos, int hp) {
    enemyHP_[pos.x][pos.y] = hp;
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

  // 移动僵尸
  void moveGhosts() {
    for (auto& ghost : ghosts_) {
      if (!ghost.active) {
        continue;
      }

      if (ghost.pos.distanceTo(playerPos_) < 3) {
        setElement(ghost.pos, MapElement::Empty);
        Position newPos = ghost.pos;

        if (playerPos_.x < ghost.pos.x && isFloorElement(getElement({ghost.pos.x - 1, ghost.pos.y}))) {
          newPos.x--;
        } else if (playerPos_.x > ghost.pos.x && isFloorElement(getElement({ghost.pos.x + 1, ghost.pos.y}))) {
          newPos.x++;
        } else if (playerPos_.y < ghost.pos.y && isFloorElement(getElement({ghost.pos.x, ghost.pos.y - 1}))) {
          newPos.y--;
        } else if (playerPos_.y > ghost.pos.y && isFloorElement(getElement({ghost.pos.x, ghost.pos.y + 1}))) {
          newPos.y++;
        }

        ghost.pos = newPos;
        setElement(ghost.pos, ghost.type);
        setEnemyHP(ghost.pos, ghost.hp);
      }
    }
  }

  // 移动骷髅
  void moveSkeletons() {
    for (auto& skeleton : skeletons_) {
      if (!skeleton.active) {
        continue;
      }

      if (skeleton.pos != playerPos_) {
        setElement(skeleton.pos, MapElement::Empty);
        Position newPos = skeleton.pos;

        if (skeleton.pos.x > playerPos_.x && isFloorElement(getElement({skeleton.pos.x - 1, skeleton.pos.y}))) {
          newPos.x--;
        } else if (skeleton.pos.x < playerPos_.x && isFloorElement(getElement({skeleton.pos.x + 1, skeleton.pos.y}))) {
          newPos.x++;
        } else if (skeleton.pos.y > playerPos_.y && isFloorElement(getElement({skeleton.pos.x, skeleton.pos.y - 1}))) {
          newPos.y--;
        } else if (skeleton.pos.y < playerPos_.y && isFloorElement(getElement({skeleton.pos.x, skeleton.pos.y + 1}))) {
          newPos.y++;
        }

        skeleton.pos = newPos;
        setElement(skeleton.pos, skeleton.type);
        setEnemyHP(skeleton.pos, skeleton.hp);
      }
    }
  }

  // 骷髅射箭
  void skeletonsShootArrows() {
    for (auto const& skeleton : skeletons_) {
      if (!skeleton.active) {
        continue;
      }

      // 检查是否在同一行或列
      if (skeleton.pos.x == playerPos_.x) {
        if (skeleton.pos.y < playerPos_.y && isFloorElement(getElement({skeleton.pos.x, skeleton.pos.y + 1}))) {
          createArrow({skeleton.pos.x, skeleton.pos.y + 1}, 0);  // 向右
        } else if (skeleton.pos.y > playerPos_.y && isFloorElement(getElement({skeleton.pos.x, skeleton.pos.y - 1}))) {
          createArrow({skeleton.pos.x, skeleton.pos.y - 1}, 2);  // 向左
        }
      } else if (skeleton.pos.y == playerPos_.y) {
        if (skeleton.pos.x < playerPos_.x && isFloorElement(getElement({skeleton.pos.x + 1, skeleton.pos.y}))) {
          createArrow({skeleton.pos.x + 1, skeleton.pos.y}, 1);  // 向下
        } else if (skeleton.pos.x > playerPos_.x && isFloorElement(getElement({skeleton.pos.x - 1, skeleton.pos.y}))) {
          createArrow({skeleton.pos.x - 1, skeleton.pos.y}, 3);  // 向上
        }
      }
    }
  }

  // 创建箭矢
  void createArrow(Position const& pos, int direction) {
    if (arrowCount_ < MAX_ARROWS) {
      for (auto& arrow : arrows_) {
        if (!arrow.active) {
          arrow.pos       = pos;
          arrow.direction = direction;
          arrow.active    = true;
          setElement(pos, MapElement::Arrow);
          arrowCount_++;
          break;
        }
      }
    }
  }

  // 移动箭矢
  void moveArrows() {
    for (auto& arrow : arrows_) {
      if (!arrow.active) {
        continue;
      }

      setElement(arrow.pos, MapElement::Empty);
      Position newPos = arrow.pos + DIRECTIONS[arrow.direction];

      if (isInBounds(newPos) && isFloorElement(getElement(newPos))) {
        arrow.pos = newPos;
        setElement(arrow.pos, MapElement::Arrow);
      } else {
        arrow.active = false;
        arrowCount_--;
      }
    }
  }

  // 检查僵尸攻击
  [[nodiscard]] int checkGhostAttacks() const {
    int damage = 0;
    for (auto const& ghost : ghosts_) {
      if (!ghost.active) {
        continue;
      }
      for (auto const& dir : DIRECTIONS) {
        Position checkPos = ghost.pos + dir;
        if (checkPos == playerPos_) {
          damage += 2;
        }
      }
    }
    return damage;
  }

  // 检查箭矢伤害
  [[nodiscard]] int checkArrowDamage() const {
    int damage = 0;
    for (auto const& arrow : arrows_) {
      if (!arrow.active) {
        continue;
      }
      Position nextPos = arrow.pos + DIRECTIONS[arrow.direction];
      if (nextPos == playerPos_) {
        damage += 1;
      }
    }
    return damage;
  }

  // 攻击位置的敌人
  std::pair<bool, std::optional<std::string>> attackAt(Position const& pos, GameState& state) {
    auto elem = getElement(pos);
    int hp    = getEnemyHP(pos);

    // Ghost 系
    if (elem == MapElement::Ghost || elem == MapElement::GhostHurt || elem == MapElement::GhostWeak) {
      hp -= PLAYER_DAMAGE;
      setEnemyHP(pos, hp);

      if (hp <= 0) {
        setElement(pos, MapElement::Empty);
        state.ghostsDefeated++;

        for (auto& ghost : ghosts_) {
          if (ghost.active && ghost.pos == pos) {
            ghost.active = false;
            break;
          }
        }

        int meatDrop        = rng_.getInt(0, 6);
        state.backpack[11] += meatDrop;

        std::stringstream msg;
        msg << "击败僵尸！获得腐肉 x" << meatDrop;
        return {true, msg.str()};
      }

      MapElement newType = (hp == 2) ? MapElement::GhostHurt : MapElement::GhostWeak;
      setElement(pos, newType);

      for (auto& ghost : ghosts_) {
        if (ghost.active && ghost.pos == pos) {
          ghost.hp   = hp;
          ghost.type = newType;
          break;
        }
      }

      return {false, std::nullopt};
    }

    // Skeleton 系
    if (elem == MapElement::Skeleton || elem == MapElement::SkeletonHurt || elem == MapElement::SkeletonWeak) {
      hp -= PLAYER_DAMAGE;
      setEnemyHP(pos, hp);

      if (hp <= 0) {
        setElement(pos, MapElement::Empty);
        state.skeletonsDefeated++;

        for (auto& skeleton : skeletons_) {
          if (skeleton.active && skeleton.pos == pos) {
            skeleton.active = false;
            break;
          }
        }

        int boneDrop        = rng_.getInt(0, 4);
        state.backpack[12] += boneDrop;

        std::stringstream msg;
        msg << "击败骷髅！获得骨头 x" << boneDrop;
        return {true, msg.str()};
      }

      MapElement newType = (hp == 2) ? MapElement::SkeletonHurt : MapElement::SkeletonWeak;
      setElement(pos, newType);

      for (auto& skeleton : skeletons_) {
        if (skeleton.active && skeleton.pos == pos) {
          skeleton.hp   = hp;
          skeleton.type = newType;
          break;
        }
      }

      return {false, std::nullopt};
    }

    return {false, std::nullopt};
  }

  // 使用喷溅型伤害药水
  void useSplashPotion(Position const& center, GameState& state) {
    for (int i = std::max(1, center.x - 2); i <= std::min(size_, center.x + 2); ++i) {
      for (int j = std::max(1, center.y - 2); j <= std::min(size_, center.y + 2); ++j) {
        Position checkPos(i, j);
        auto elem = getElement(checkPos);

        if (elem == MapElement::Ghost || elem == MapElement::GhostHurt || elem == MapElement::GhostWeak) {
          setElement(checkPos, MapElement::Empty);
          state.ghostsDefeated++;
          for (auto& ghost : ghosts_) {
            if (ghost.active && ghost.pos == checkPos) {
              ghost.active = false;
              break;
            }
          }
        } else if (elem == MapElement::Skeleton || elem == MapElement::SkeletonHurt
                   || elem == MapElement::SkeletonWeak) {
          setElement(checkPos, MapElement::Empty);
          state.skeletonsDefeated++;
          for (auto& sk : skeletons_) {
            if (sk.active && sk.pos == checkPos) {
              sk.active = false;
              break;
            }
          }
        }
      }
    }
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
  static bool isFloorElement(MapElement e) noexcept {
    return static_cast<int>(e) < static_cast<int>(MapElement::WallStart);
  }

  void resetMap() {
    for (auto& row : map_) {
      std::fill(row.begin(), row.end(), MapElement::Empty);
    }
    for (auto& row : enemyHP_) {
      std::fill(row.begin(), row.end(), 0);
    }
    for (auto& ghost : ghosts_) {
      ghost.active = false;
    }
    for (auto& skeleton : skeletons_) {
      skeleton.active = false;
    }
    for (auto& arrow : arrows_) {
      arrow.active = false;
    }
    arrowCount_ = 0;
    resetVisited();
  }

  void resetVisited() {
    for (auto& row : visited_) {
      std::fill(row.begin(), row.end(), false);
    }
  }

  void generateRecursive(int x, int y) {
    visited_[x][y] = true;
    auto value     = rng_.getInt(0, 9);
    map_[x][y]     = static_cast<MapElement>(value);

    for (auto const& dir : DIRECTIONS) {
      int nextX = x + dir.dx, nextY = y + dir.dy;
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
        && map_[x][y] != MapElement::Ghost && map_[x][y] != MapElement::GhostHurt && map_[x][y] != MapElement::GhostWeak
        && map_[x][y] != MapElement::Skeleton && map_[x][y] != MapElement::SkeletonHurt
        && map_[x][y] != MapElement::SkeletonWeak) {
      return false;
    }

    visited_[x][y] = true;

    return std::any_of(DIRECTIONS.begin(), DIRECTIONS.end(), [&](Direction const& dir) {
      int nx = x + dir.dx, ny = y + dir.dy;
      return isInBounds({nx, ny}) && !visited_[nx][ny]
             && (static_cast<int>(map_[nx][ny]) < static_cast<int>(MapElement::WallStart)
                 || map_[nx][ny] == MapElement::Door || map_[nx][ny] == MapElement::Ghost
                 || map_[nx][ny] == MapElement::GhostHurt || map_[nx][ny] == MapElement::GhostWeak
                 || map_[nx][ny] == MapElement::Skeleton || map_[nx][ny] == MapElement::SkeletonHurt
                 || map_[nx][ny] == MapElement::SkeletonWeak)
             && checkPath(nx, ny);
    });
  }

  // 避免在出生点(2,2)周围一圈生成敌人
  void placeGhosts() {
    int ghostCount = 3, placed = 0;
    Position spawn(2, 2);

    for (int i = 0; i < ghostCount && placed < MAX_ENEMIES; ++i) {
      auto x = rng_.getInt(1, size_ + 1);
      auto y = rng_.getInt(1, size_ + 1);
      Position p(x, y);

      if (static_cast<int>(map_[x][y]) < static_cast<int>(MapElement::WallStart) && (x != 2 || y != 2)
          && (x != size_ - 1 || y != size_ - 1) && p.distanceTo(spawn) > 1) {
        map_[x][y]     = MapElement::Ghost;
        enemyHP_[x][y] = GHOST_MAX_HP;

        ghosts_[placed].pos    = p;
        ghosts_[placed].hp     = GHOST_MAX_HP;
        ghosts_[placed].type   = MapElement::Ghost;
        ghosts_[placed].active = true;
        placed++;
      }
    }
  }

  // 避免在出生点周围一圈生成敌人
  void placeSkeletons() {
    int skeletonCount = 2, placed = 0;
    Position spawn(2, 2);

    for (int i = 0; i < skeletonCount && placed < MAX_ENEMIES; ++i) {
      auto x = rng_.getInt(1, size_ + 1);
      auto y = rng_.getInt(1, size_ + 1);
      Position p(x, y);

      if (static_cast<int>(map_[x][y]) < static_cast<int>(MapElement::WallStart) && (x != 2 || y != 2)
          && (x != size_ - 1 || y != size_ - 1) && p.distanceTo(spawn) > 1) {
        map_[x][y]     = MapElement::Skeleton;
        enemyHP_[x][y] = SKELETON_MAX_HP;

        skeletons_[placed].pos    = p;
        skeletons_[placed].hp     = SKELETON_MAX_HP;
        skeletons_[placed].type   = MapElement::Skeleton;
        skeletons_[placed].active = true;
        placed++;
      }
    }
  }

  // 避免在出生点周围一圈生成宝箱
  void placeChests() {
    int chestCount = rng_.getInt(1, size_);
    Position spawn(2, 2);

    for (int i = 0; i < chestCount; ++i) {
      auto x = rng_.getInt(1, size_ + 1);
      auto y = rng_.getInt(1, size_ + 1);
      Position p(x, y);

      if (static_cast<int>(map_[x][y]) < static_cast<int>(MapElement::WallStart) && (x != 2 || y != 2)
          && (x != size_ - 1 || y != size_ - 1) && p.distanceTo(spawn) > 1) {
        map_[x][y] = MapElement::Chest;
      }
    }
  }

  [[nodiscard]] char getDisplayChar(Position const& pos) const {
    auto element = map_[pos.x][pos.y];

    if (element == MapElement::Ghost || element == MapElement::GhostHurt || element == MapElement::GhostWeak) {
      int hp = enemyHP_[pos.x][pos.y];
      if (hp == 3) {
        return 'Z';
      }
      if (hp == 2) {
        return '2';
      }
      if (hp == 1) {
        return '1';
      }
    }

    if (element == MapElement::Skeleton || element == MapElement::SkeletonHurt || element == MapElement::SkeletonWeak) {
      int hp = enemyHP_[pos.x][pos.y];
      if (hp == 3) {
        return 'S';
      }
      if (hp == 2) {
        return '2';
      }
      if (hp == 1) {
        return '1';
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
      case MapElement::Arrow:
        return 'A';
      default:
        return '?';
    }
  }
};

// ==================== 商店系统 ====================
class Shop {
 public:
  Shop() = default;

  static void enter(GameState& state) {
    while (true) {
      Platform::clearScreen();
      displayMenu(state);

      int choice;
      std::cout << "选择操作 (0-2): ";
      std::cin >> choice;

      switch (choice) {
        case 0:
          buyItems(state);
          break;
        case 1:
          sellItems(state);
          break;
        case 2:
          return;
        default:
          std::cout << "无效选择！\n";
          Platform::pause(std::chrono::milliseconds(1000));
      }
    }
  }

 private:
  static void displayMenu(GameState const& state) {
    Ui::printTop();
    Ui::printLine("商店");
    Ui::printMid();
    Ui::printKV("当前金币", std::to_string(state.money) + " 枚");
    Ui::printMid();
    Ui::printLine("[0] 购买物品");
    Ui::printLine("[1] 出售物品");
    Ui::printLine("[2] 离开商店");
    Ui::printBottom();
  }

  static void buyItems(GameState& state) {
    Platform::clearScreen();
    Ui::printTop();
    Ui::printLine("购买物品");
    Ui::printMid();
    Ui::printKV("金币", std::to_string(state.money) + " 枚");
    Ui::printMid();
    Ui::printLine("1. 末影珍珠: $100,000 (传送至附近9x9范围)");
    Ui::printLine("2. 治疗药水: $500,000 (恢复至满血)");
    Ui::printLine("3. 喷溅型伤害药水: $300,000 (5x5范围伤害)");
    Ui::printLine("4. 面包: $10 (恢复3点饱食度)");
    Ui::printBottom();

    int itemId, quantity;
    std::cout << "购买物品编号(1-4)和数量: ";
    std::cin >> itemId >> quantity;

    if (itemId < 1 || itemId > 4 || quantity <= 0) {
      std::cout << "无效输入！\n";
      Platform::pause(std::chrono::milliseconds(1000));
      return;
    }

    int itemIndex = -1;
    switch (itemId) {
      case 1:
        itemIndex = 8;
        break;  // 末影珍珠
      case 2:
        itemIndex = 9;
        break;  // 治疗药水
      case 3:
        itemIndex = 10;
        break;  // 喷溅型伤害药水
      case 4:
        itemIndex = 13;
        break;  // 面包
      default:
        break;
    }

    if (itemIndex == -1) {
      std::cout << "无效输入！\n";
      Platform::pause(std::chrono::milliseconds(1000));
      return;
    }

    int cost = ITEMS[itemIndex].value * quantity;

    if (state.money >= cost) {
      state.money               -= cost;
      state.backpack[itemIndex] += quantity;
      std::cout << "购买成功！获得 " << ITEMS[itemIndex].name << " x" << quantity << "\n";
    } else {
      std::cout << "金币不足！需要 " << cost << " 金币\n";
    }

    Platform::pause(std::chrono::milliseconds(2000));
  }

  static void sellItems(GameState& state) {
    Platform::clearScreen();
    Ui::printTop();
    Ui::printLine("出售物品");
    Ui::printMid();

    int totalValue = 0;
    for (size_t i = 0; i < 8; ++i) {  // 只出售前8种物品
      if (state.backpack[i] > 0 && ITEMS[i].sellable) {
        int value   = state.backpack[i] * ITEMS[i].value;
        totalValue += value;
        Ui::printKV(std::string(ITEMS[i].name),
                    std::to_string(state.backpack[i]) + " 个 (价值: " + std::to_string(value) + " 金币)");
      }
    }

    Ui::printMid();
    Ui::printKV("总价值", std::to_string(totalValue) + " 金币");
    Ui::printBottom();

    if (totalValue > 0) {
      std::cout << "\n确认出售所有物品？(Y/N): ";
      char confirm;
      std::cin >> confirm;

      if (std::tolower(static_cast<unsigned char>(confirm)) == 'y') {
        state.money += totalValue;
        for (size_t i = 0; i < 8; ++i) {
          state.backpack[i] = 0;
        }
        std::cout << "成交！获得 " << totalValue << " 金币\n";
      }
    } else {
      std::cout << "\n没有可出售的物品！\n";
    }

    Platform::pause(std::chrono::milliseconds(2000));
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
    Ui::printTop();
    Ui::printKV("金钱", std::to_string(state.money));
    Ui::printKV("生命", std::to_string(state.healthPoints));
    Ui::printKV("饱食度", std::to_string(state.fullness));
    Ui::printBottom();
  }

  static void displayBackpack(GameState const& state) {
    Platform::clearScreen();

    Ui::printTop();
    Ui::printLine("背包");
    Ui::printMid();

    for (size_t i = 0; i < ITEM_TYPES; ++i) {
      Ui::printKV((std::to_string(i + 1) + ". " + std::string(ITEMS[i].name)),
                  std::to_string(state.backpack[i]) + " 个");
    }

    Ui::printBottom();
    std::cout << "\n按任意键返回...\n";
    Platform::getChar();
  }

  static void displayChestOpening(int totalValue, std::array<int, ITEM_TYPES> const& items) {
    Platform::clearScreen();

    // 简单动画
    std::cout << "\n\n                    正在打开宝箱...\n";
    std::cout << "                         ___\n";
    std::cout << "                        /   \\\n";
    std::cout << "                       /_____\\\n";
    std::cout << "                      [=======]\n";
    std::cout << "                      [_______]\n";
    Platform::pause(std::chrono::milliseconds(500));

    Platform::clearScreen();
    Ui::printTop();
    Ui::printLine("开启宝箱");
    Ui::printMid();

    bool hasLoot = false;
    for (size_t i = 0; i < 8; ++i) {  // 只显示前8种可售物品
      if (items[i] > 0) {
        hasLoot = true;
        Ui::printKV(std::string(ITEMS[i].name), "x" + std::to_string(items[i]));
      }
    }

    if (!hasLoot) {
      Ui::printLine("宝箱是空的，真不走运！");
    }

    Ui::printMid();
    Ui::printKV("战利品总价值", std::to_string(totalValue) + " 金币");
    Ui::printBottom();

    std::cout << "\n按任意键继续...\n";
    Platform::getChar();
  }

  static void displayHelp() {
    Platform::clearScreen();
    Ui::printTop();
    Ui::printLine("快速帮助");
    Ui::printMid();
    Ui::printLine("操作说明：");
    Ui::printListItem("W/A/S/D", "上/左/下/右 移动");
    Ui::printListItem("O", "开启宝箱");
    Ui::printListItem("K", "攻击敌人");
    Ui::printListItem("E", "查看背包");
    Ui::printListItem("U", "使用道具");
    Ui::printListItem("Q", "食用食物");
    Ui::printListItem("T 或 ?", "显示帮助");
    Ui::printListItem("R", "退出游戏");
    Ui::printMid();
    Ui::printLine("敌人系统：");
    Ui::printSymbolItem("Z/2/1", "僵尸", "3血，近战，掉腐肉");
    Ui::printSymbolItem("S/2/1", "骷髅", "3血，远程，掉骨头");
    Ui::printSymbolItem("A", "箭矢", "骷髅射击，1点伤害");
    Ui::printBottom();
    std::cout << "\n按任意键返回游戏...\n";
    Platform::getChar();
  }

  static void displayItemUse(GameState const& state) {
    Platform::clearScreen();
    Ui::printTop();
    Ui::printLine("使用道具");
    Ui::printMid();
    Ui::printListItem("1. 末影珍珠", std::to_string(state.backpack[8]) + " 个 (传送9x9)");
    Ui::printListItem("2. 治疗药水", std::to_string(state.backpack[9]) + " 个 (满血恢复)");
    Ui::printListItem("3. 喷溅伤害药水", std::to_string(state.backpack[10]) + " 个 (5x5伤害)");
    Ui::printBottom();
  }

  static void displayFood(GameState const& state) {
    Platform::clearScreen();
    Ui::printTop();
    Ui::printLine("食物");
    Ui::printMid();
    Ui::printListItem("1. 腐肉", std::to_string(state.backpack[11]) + " 个 (+1饱食度/个)");
    Ui::printListItem("2. 面包", std::to_string(state.backpack[13]) + " 个 (+3饱食度/个)");
    Ui::printBottom();
  }

  static void displayTutorial() {
    Ui::printTop();
    Ui::printLine("游戏教程");
    Ui::printMid();
    Ui::printLine("目标：从起点到达出口，收集财宝！");
    Ui::printMid();
    Ui::printLine("地图符号：");
    Ui::printSymbolItem(" ", "空地", "可以通行");
    Ui::printSymbolItem("W", "墙壁", "不可通过");
    Ui::printSymbolItem("C", "宝箱", "包含宝物");
    Ui::printSymbolItem("Y", "玩家", "你的位置");
    Ui::printSymbolItem("Z", "僵尸", "近战敌人");
    Ui::printSymbolItem("S", "骷髅", "远程敌人");
    Ui::printSymbolItem("A", "箭矢", "飞行攻击");
    Ui::printSymbolItem("D", "出口", "目标位置");
    Ui::printMid();
    Ui::printLine("游戏提示：");
    Ui::printListItem("饱食度每10步减少1点");
    Ui::printListItem("饱食度20时缓慢回血");
    Ui::printListItem("饱食度0时持续掉血");
    Ui::printListItem("击败敌人可获得食物");
    Ui::printListItem("注意躲避骷髅的箭矢");
    Ui::printBottom();
    std::cout << "\n按任意键开始游戏...";
    Platform::getChar();
  }
};

// ==================== 游戏引擎 ====================
class Game {
 private:
  GameState state_;
  std::unique_ptr<GameMap> map_;
  std::unique_ptr<Shop> shop_;
  UIManager ui_;
  RandomGenerator rng_;
  int mapSize_;
  int lastDirection_ = 0;  // 记录上次移动方向

 public:
  Game() : shop_(std::make_unique<Shop>()), mapSize_(10) {}

  void run() {
    Platform::initConsole();
    Platform::setColor();
    displayTitle();

    std::cout << "请输入地图大小 (5-20): ";
    std::cin >> mapSize_;
    mapSize_ = std::clamp(mapSize_, 5, 20);

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
      // 更新饱食度和生命值
      state_.updateFullness();
      state_.updateHealth();

      Platform::clearScreen();

      // 检查游戏状态
      if (state_.healthPoints <= 0) {
        return displayDeath();
      }

      // 检查是否到达终点
      if (isAtExit()) {
        return displayVictory();
      }

      // 移动敌人
      map_->moveGhosts();
      map_->moveSkeletons();
      map_->skeletonsShootArrows();
      map_->moveArrows();

      // 显示状态与地图
      UIManager::displayGameStatus(state_);
      map_->display();
      std::cout << "==================================================\n";
      std::cout << "敌人: [Z/2/1]=僵尸 [S/2/1]=骷髅 [A]=箭矢\n";
      std::cout << "操作: W/A/S/D移动 | O开箱 | K攻击 | E背包 | U道具 | Q食物\n";
      std::cout << "      T帮助 | R退出\n";

      // 检查敌人攻击
      int damage = map_->checkGhostAttacks();
      if (damage > 0) {
        state_.healthPoints -= damage;
        std::cout << ">> 被僵尸攻击！受到 " << damage << " 点伤害\n";
      }

      damage = map_->checkArrowDamage();
      if (damage > 0) {
        state_.healthPoints -= damage;
        std::cout << ">> 被箭矢击中！受到 " << damage << " 点伤害\n";
      }

      // 显示消息
      auto msg = ui_.getAndClearMessage();
      if (!msg.empty()) {
        std::cout << ">> " << msg << '\n';
      }

      if (!processInput()) {
        return false;
      }
    }
  }

  bool processInput() {
    char input = Platform::getChar();
    input      = static_cast<char>(std::tolower(static_cast<unsigned char>(input)));

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
        attackEnemy();
        break;
      case 'e':
        UIManager::displayBackpack(state_);
        break;
      case 'u':
        useItem();
        break;
      case 'q':
        eatFood();
        break;
      case 't':
      case '?':
        UIManager::displayHelp();
        break;
      case 'r':
        std::cout << "\n确定要退出游戏吗？(Y/N): ";
        if (std::tolower(static_cast<unsigned char>(Platform::getChar())) == 'y') {
          return false;
        }
        break;
      default:
        break;
    }

    return true;
  }

  void handleMovement(char direction) {
    auto currentPos    = map_->getPlayerPos();
    Position targetPos = currentPos;

    switch (direction) {
      case 'w':
        targetPos      = currentPos + DIRECTIONS[3];
        lastDirection_ = 3;
        break;
      case 'a':
        targetPos      = currentPos + DIRECTIONS[2];
        lastDirection_ = 2;
        break;
      case 's':
        targetPos      = currentPos + DIRECTIONS[1];
        lastDirection_ = 1;
        break;
      case 'd':
        targetPos      = currentPos + DIRECTIONS[0];
        lastDirection_ = 0;
        break;
      default:
        return;
    }

    if (!map_->isInBounds(targetPos)) {
      ui_.showMessage("无法移动到该位置！");
      return;
    }

    auto targetElement = map_->getElement(targetPos);

    if (static_cast<int>(targetElement) < static_cast<int>(MapElement::WallStart) || targetElement == MapElement::Door
        || targetElement == MapElement::Arrow) {
      map_->setElement(currentPos, MapElement::Empty);
      map_->setElement(targetPos, MapElement::Player);
      map_->setPlayerPos(targetPos);
      state_.steps++;
    } else {
      ui_.showMessage("无法移动到该位置！");
    }
  }

  // 开箱：先按上次方向尝试，否则自动检查四个方向
  void openChest() {
    auto playerPos = map_->getPlayerPos();

    std::array<int, 4> order
        = {lastDirection_, (lastDirection_ + 1) % 4, (lastDirection_ + 3) % 4, (lastDirection_ + 2) % 4};

    std::optional<Position> found;
    for (int dirIdx : order) {
      Position p = playerPos + DIRECTIONS[dirIdx];
      if (map_->isInBounds(p) && map_->getElement(p) == MapElement::Chest) {
        found = p;
        break;
      }
    }

    if (!found.has_value()) {
      ui_.showMessage("附近没有宝箱！");
      return;
    }

    Position chestPos = *found;
    state_.chestsOpened++;
    map_->setElement(chestPos, MapElement::Empty);

    // 生成战利品
    std::array<int, ITEM_TYPES> newItems{};
    int totalValue = 0;

    // 原石
    newItems[0] = rng_.getInt(0, 20);
    // 煤炭
    newItems[1] = rng_.getInt(0, 64);
    // 铁锭和金锭
    newItems[2] = rng_.getInt(0, 20);
    newItems[3] = rng_.getInt(0, 20);
    // 红石和青金石
    newItems[4] = rng_.getInt(0, 10);
    newItems[5] = rng_.getInt(0, 10);
    // 绿宝石和钻石
    newItems[6] = rng_.getInt(0, 3);
    newItems[7] = rng_.getInt(0, 3);

    for (size_t i = 0; i < 8; ++i) {
      if (newItems[i] > 0) {
        state_.backpack[i] += newItems[i];
        totalValue         += newItems[i] * ITEMS[i].value;
      }
    }

    state_.money += totalValue;
    UIManager::displayChestOpening(totalValue, newItems);
  }

  void attackEnemy() {
    auto playerPos = map_->getPlayerPos();
    bool attacked  = false;

    for (auto const& dir : DIRECTIONS) {
      Position checkPos = playerPos + dir;
      if (!map_->isInBounds(checkPos)) {
        continue;
      }

      auto [killed, message] = map_->attackAt(checkPos, state_);

      if (message.has_value()) {
        ui_.showMessage(message.value());
        attacked = true;
        break;
      }
    }

    if (!attacked) {
      ui_.showMessage("周围没有敌人！");
    }
  }

  void useItem() {
    UIManager::displayItemUse(state_);

    std::cout << "使用哪个道具 (1-3, 0取消): ";
    int choice;
    std::cin >> choice;

    switch (choice) {
      case 1:  // 末影珍珠
        if (state_.backpack[8] > 0) {
          state_.backpack[8]--;

          std::cout << "传送到 (x y): ";
          int tx, ty;
          std::cin >> tx >> ty;

          auto playerPos = map_->getPlayerPos();
          tx             = std::clamp(tx, playerPos.x - 4, playerPos.x + 4);
          ty             = std::clamp(ty, playerPos.y - 4, playerPos.y + 4);

          Position newPos(tx, ty);
          if (map_->isInBounds(newPos) && map_->getElement(newPos) == MapElement::Empty) {
            map_->setElement(playerPos, MapElement::Empty);
            map_->setElement(newPos, MapElement::Player);
            map_->setPlayerPos(newPos);
            ui_.showMessage("传送成功！");
          } else {
            state_.backpack[8]++;  // 传送失败，返还物品
            ui_.showMessage("无法传送到该位置！");
          }
        } else {
          ui_.showMessage("没有末影珍珠！");
        }
        break;

      case 2:  // 治疗药水
        if (state_.backpack[9] > 0) {
          state_.backpack[9]--;
          state_.healthPoints = INITIAL_HEALTH;
          ui_.showMessage("生命值恢复至满！");
        } else {
          ui_.showMessage("没有治疗药水！");
        }
        break;

      case 3:  // 喷溅型伤害药水
        if (state_.backpack[10] > 0) {
          state_.backpack[10]--;
          map_->useSplashPotion(map_->getPlayerPos(), state_);
          ui_.showMessage("使用喷溅型伤害药水！5x5范围内的敌人被消灭！");
        } else {
          ui_.showMessage("没有喷溅型伤害药水！");
        }
        break;

      default:
        break;
    }
  }

  void eatFood() {
    UIManager::displayFood(state_);

    std::cout << "吃哪种食物，吃多少 (食物编号 数量): ";
    int foodType, amount;
    std::cin >> foodType >> amount;

    if (foodType == 1 && state_.backpack[11] > 0) {  // 腐肉
      amount               = std::min(amount, state_.backpack[11]);
      state_.backpack[11] -= amount;
      state_.fullness      = std::min(INITIAL_FULLNESS, state_.fullness + (amount * 1));
      ui_.showMessage("吃了腐肉，恢复饱食度！");
    } else if (foodType == 2 && state_.backpack[13] > 0) {  // 面包
      amount               = std::min(amount, state_.backpack[13]);
      state_.backpack[13] -= amount;
      state_.fullness      = std::min(INITIAL_FULLNESS, state_.fullness + (amount * 3));
      ui_.showMessage("吃了面包，恢复饱食度！");
    } else {
      ui_.showMessage("没有该食物或数量不足！");
    }
  }

  [[nodiscard]] bool isAtExit() const {
    auto pos = map_->getPlayerPos();
    return pos.x == mapSize_ - 1 && pos.y == mapSize_ - 1;
  }

  bool displayVictory() {
    Platform::clearScreen();
    Ui::printTop();
    Ui::printLine("666，你到了终点!");
    Ui::printMid();
    Ui::printLine("[0] 退出");
    Ui::printLine("[1] 继续");
    Ui::printLine("[2] 进入商店");
    Ui::printBottom();

    int choice;
    std::cin >> choice;

    switch (choice) {
      case 0:
        return false;
      case 1:
        state_.healthPoints = INITIAL_HEALTH;
        state_.fullness     = INITIAL_FULLNESS;
        return true;
      case 2:
        ChestsInMaps::Shop::enter(state_);
        state_.healthPoints = INITIAL_HEALTH;
        state_.fullness     = INITIAL_FULLNESS;
        return true;
      default:
        return false;
    }
  }

  bool displayDeath() {
    Platform::clearScreen();
    Platform::setColor();
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xCF);

    Ui::printTop();
    Ui::printLine("你死了！");
    Ui::printMid();
    Ui::printLine("[0] 退出游戏");
    Ui::printLine("[1] 重生");
    Ui::printLine("[2] 返回标题屏幕");
    Ui::printBottom();

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x03);

    int choice;
    std::cin >> choice;

    switch (choice) {
      case 0:
        return false;
      case 1:
        state_.reset();
        Platform::setColor();
        return true;
      case 2:
        run();
        return false;
      default:
        return false;
    }
  }

  void displayGameOver() const {
    Platform::clearScreen();
    Ui::printTop();
    Ui::printLine("游戏结束！  最终战绩");
    Ui::printMid();
    Ui::printKV("总金币", std::to_string(state_.money) + " 枚");
    Ui::printKV("开启宝箱", std::to_string(state_.chestsOpened) + " 个");
    Ui::printKV("击败僵尸", std::to_string(state_.ghostsDefeated) + " 个");
    Ui::printKV("击败骷髅", std::to_string(state_.skeletonsDefeated) + " 个");
    Ui::printBottom();
    std::cout << "\n感谢游玩！下次再见！\n";
    Platform::pause(std::chrono::milliseconds(2000));
  }

  static void displayTitle() {
    Ui::printTitleBox("CHESTS IN MAPS | 宝箱探险游戏", {"C++ Canary Edition", "Version: C++ Canary Edition 5.0"});
    std::cout << "\n按任意键开始游戏...\n";
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