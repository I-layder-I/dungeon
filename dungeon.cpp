#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <ncurses.h>
#include <string>
#include <vector>

using namespace std;

struct GameState;
void draw_screen(GameState &g);
void screen(bool state);

class Player {

  void dash(int input, GameState &g);

public:
  int x, y;
  int gold = 0, monsters_killed = 0, dashCount = 0;

  void move(int input, GameState &g);
};

class Monsters {

  int maxMonsters = 15;
  int currentMonsters = 0;

public:
  void spawnMonsters(GameState &g);
  void killMonster(GameState &g, int y, int x);
};

struct GameState {
  int rows = 0, cols = 0, old_rows = -1, old_cols = -1;
  vector<string> map;

  Player player;
  Monsters monsters;
};

struct Room {
  int id;
  int x;
  int y;
  int height;
  int width;

  vector<int> connections;
};

int main() {

  screen(1);

  int input = 0;

  GameState game;

  while (input != 27) {

    draw_screen(game);

    game.player.move(input = getch(), game);
  }

  screen(0);

  return 0;
}

class DungeonGenerator {
  vector<Room> rooms;
  const int border = 2;

  void create_room(GameState &g) {
    const int rows = g.rows;
    const int cols = g.cols;
    vector<string> &map = g.map;

    int x, y;
    int height, width;

    bool invalid_position = false;
    Room room;

    do {
      invalid_position = false;

      height = rand() % 6 + 3;
      width = rand() % 6 + 3;

      y = rand() % (rows - 2 * height / 2 - 2 * border) + height / 2 + border;
      x = rand() % (cols - 2 * width / 2 - 2 * border) + width / 2 + border;

      room.id = rooms.size() + 1;
      room.x = x;
      room.y = y;
      room.width = width;
      room.height = height;

      if (rand() % 100 < 30) {
        for (int i = 0; i < rooms.size(); i++)
          if (intersects(room, rooms[i])) {
            invalid_position = true;
            break;
          }
      }
    } while (invalid_position);
    for (int iy = y - height / 2; iy <= y + height / 2; iy++) {
      for (int ix = x - width / 2; ix <= x + width / 2; ix++) {
        map[iy][ix] = ' ';
      }
    }

    rooms.push_back(room);
  }

  bool intersects(const Room &a, const Room &b) {
    int padding = rand() % 4 + 1;
    int a_left = a.x - a.width / 2 - padding;
    int a_right = a.x + a.width / 2 + padding;
    int a_top = a.y - a.height / 2 - padding;
    int a_bottom = a.y + a.height / 2 + padding;

    int b_left = b.x - b.width / 2 - padding;
    int b_right = b.x + b.width / 2 + padding;
    int b_top = b.y - b.height / 2 - padding;
    int b_bottom = b.y + b.height / 2 + padding;

    return !(a_right < b_left || a_left > b_right || a_bottom < b_top ||
             a_top > b_bottom);
  }

  void create_corridor(GameState &g, Room &a, Room &b) {
    // горизонталь до новой комнаты
    int x = a.x, y = a.y;
    vector<string> &map = g.map;

    while (x != b.x) {
      if (x < b.x)
        x++;
      else
        x--;
      map[y][x] = ' ';
    }

    // вертикаль до новой комнаты
    while (y != b.y) {
      if (y < b.y)
        y++;
      else
        y--;
      map[y][x] = ' ';
    }

    a.connections.push_back(b.id);
    b.connections.push_back(a.id);
  }

public:
  void generate(GameState &g) {
    vector<string> &map = g.map;
    int &rows = g.rows;
    int &cols = g.cols;

    rooms.clear();

    const int max_rooms = rand() % 7 + 15;

    const int min_corridors = max_rooms - 1;
    const int max_corridors = (max_rooms * (max_rooms - 1)) / 2;

    const int corridor_count =
        rand() % (max_corridors - min_corridors + 1) + min_corridors;

    const int additional_corridors = corridor_count - min_corridors;

    // Заполняем карту стенами
    for (int y = 0; y < rows; y++) {
      for (int x = 0; x < cols; x++) {
        if (y < border || y >= rows - border || x < border ||
            x >= cols - border) {
          map[y][x] = '%';
        } else {
          map[y][x] = '#';
        }
      }
    }

    // Создаём комнаты
    for (int i = 0; i < max_rooms; i++)
      create_room(g);

    // Гарантируем связность
    for (int i = 0; i < min_corridors; i++)
      create_corridor(g, rooms[i], rooms[i + 1]);

    // Добавляем случайные дополнительные коридоры
    for (int i = 0; i < additional_corridors; i++) {
      int a = rand() % max_rooms;
      int b = rand() % max_rooms;

      if (a == b)
        continue;

      bool already_connected = false;

      for (int connection : rooms[a].connections) {
        if (connection == rooms[b].id) {
          already_connected = true;
          break;
        }
      }

      if (already_connected)
        continue;

      create_corridor(g, rooms[a], rooms[b]);
    }
  }
};

void draw_screen(GameState &g) {

  int &rows = g.rows;
  int &cols = g.cols;
  int &old_rows = g.old_rows;
  int &old_cols = g.old_cols;
  vector<string> &map = g.map;

  getmaxyx(stdscr, rows, cols);
  if (rows != old_rows || cols != old_cols) {

    map.assign(rows, string(cols, ' '));

    DungeonGenerator generator;
    generator.generate(g);

    do {
      g.player.y = rand() % map.size();
      g.player.x = rand() % map[0].size();
    } while (map[g.player.y][g.player.x] != ' ');

    old_rows = rows;
    old_cols = cols;

    g.monsters.spawnMonsters(g);
  }

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      mvaddch(y, x, map[y][x]);
    }
  }

  mvprintw(rows - 1, 0, "Gold: %d", g.player.gold);
  mvprintw(rows - 1, 6 + to_string(g.player.gold).size() + 2,
           "Monsters killed: %d", g.player.monsters_killed);
  mvprintw(rows - 1,
           6 + to_string(g.player.gold).size() + 2 + 17 +
               to_string(g.player.monsters_killed).size() + 2,
           "Dash count: %d", g.player.dashCount);

  refresh();
}

void screen(bool state) {

  if (state) {
    srand(time(nullptr));
    initscr();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
  } else if (!state) {
    endwin();
  }
}

void Monsters::spawnMonsters(GameState &g) {

  vector<string> &map = g.map;

  int my, mx;

  for (; currentMonsters < maxMonsters; currentMonsters++) {
    do {
      my = rand() % map.size();
      mx = rand() % map[0].size();
    } while (map[my][mx] != ' ');

    map[my][mx] = 't';
  }
}

void Monsters::killMonster(GameState &g, int y, int x) {
  g.map[y][x] = ' ';

  currentMonsters--;

  g.player.gold += rand() % 21 + 5;
  if (rand() % 100 < 30)
    g.player.dashCount++;
  g.player.monsters_killed++;
};

void Player::move(int input, GameState &g) {
  vector<string> &map = g.map;

  int diry = 0, dirx = 0;

  diry = y;
  dirx = x;

  if (input == 'w')
    diry--;
  else if (input == 's')
    diry++;
  else if (input == 'a')
    dirx--;
  else if (input == 'd')
    dirx++;
  else if (input == 'W' || input == 'S' || input == 'A' || input == 'D') {
    dash(input, g);
    return;
  }

  if (map[diry][dirx] == ' ') {
    map[y][x] = ' ';
    y = diry;
    x = dirx;
  } else if (map[diry][dirx] == 't') {
    map[y][x] = ' ';
    y = diry;
    x = dirx;
    g.monsters.killMonster(g, y, x);
  }
  map[y][x] = '@';
  g.monsters.spawnMonsters(g);
};

void Player::dash(int input, GameState &g) {

  if (dashCount <= 0)
    return;

  dashCount--;

  vector<string> &map = g.map;

  int x = this->x;
  int y = this->y;

  if (input == 'W') {
    while (y > 0 && map[y - 1][x] != '#' && map[y - 1][x] != '%') {
      y--;

      if (map[y][x] == 't')
        g.monsters.killMonster(g, y, x);
    }
  }

  else if (input == 'S') {
    while (y < g.rows - 1 && map[y + 1][x] != '#' && map[y + 1][x] != '%') {
      y++;

      if (map[y][x] == 't')
        g.monsters.killMonster(g, y, x);
    }
  }

  else if (input == 'A') {
    while (x > 0 && map[y][x - 1] != '#' && map[y][x - 1] != '%') {
      x--;

      if (map[y][x] == 't')
        g.monsters.killMonster(g, y, x);
    }
  }

  else if (input == 'D') {
    while (x < g.cols - 1 && map[y][x + 1] != '#' && map[y][x + 1] != '%') {
      x++;

      if (map[y][x] == 't')
        g.monsters.killMonster(g, y, x);
    }
  }

  map[this->y][this->x] = ' ';

  this->x = x;
  this->y = y;

  map[y][x] = '@';
  // Пополняем монстров уже после всего dash
  g.monsters.spawnMonsters(g);
};
// For rand() [min; max]
// rand() % (max - min + 1) + min
