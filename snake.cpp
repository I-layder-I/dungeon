#include <cstdlib>
#include <ctime>
#include <ncurses.h>
#include <string>
#include <vector>

using namespace std;

struct GameState;
void draw_screen(GameState &g);
void spawn_monsters(GameState &g);
void movement(int input, GameState &g);
void dungeon(GameState &g);
void screen(bool state);

struct GameState {
  int rows = 0, cols = 0, old_rows = -1, old_cols = -1;
  vector<string> map;
  int player_y, player_x;
  int current_monsters = 0;
};

int main() {

  screen(1);

  int input = 0;

  GameState game;

  while (input != 27) {

    draw_screen(game);

    movement(input = getch(), game);
  }

  screen(0);

  return 0;
}

void draw_screen(GameState &g) {

  int &rows = g.rows;
  int &cols = g.cols;
  int &old_rows = g.old_rows;
  int &old_cols = g.old_cols;
  vector<string> &map = g.map;

  getmaxyx(stdscr, rows, cols);
  if (rows != old_rows || cols != old_cols) {

    map.assign(rows, string(cols, ' '));

    dungeon(g);

    do {
      g.player_y = rand() % map.size();
      g.player_x = rand() % map[0].size();
    } while (map[g.player_y][g.player_x] != ' ');

    old_rows = rows;
    old_cols = cols;

    g.current_monsters = 0;
    spawn_monsters(g);
  }

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      mvaddch(y, x, map[y][x]);
    }
  }
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

void spawn_monsters(GameState &g) {

  vector<string> &map = g.map;
  const int max_monsters = rand() % 7 + 11;

  int my, mx;

  for (; g.current_monsters < max_monsters; g.current_monsters++) {
    do {
      my = rand() % map.size();
      mx = rand() % map[0].size();
    } while (map[my][mx] != ' ');

    map[my][mx] = 't';
  }
}

void movement(int input, GameState &g) {

  vector<string> &map = g.map;
  int &player_y = g.player_y;
  int &player_x = g.player_x;

  int diry = 0, dirx = 0;
  int y = player_y, x = player_x;

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

  if (map[diry][dirx] == ' ') {
    map[y][x] = ' ';
    player_y = diry;
    player_x = dirx;
  } else if (map[diry][dirx] == 't') {
    map[y][x] = ' ';
    player_y = diry;
    player_x = dirx;
    g.current_monsters--;
    spawn_monsters(g);
  }
  map[player_y][player_x] = '@';
}

void dungeon(GameState &g) {

  vector<string> &map = g.map;
  int &rows = g.rows;
  int &cols = g.cols;

  bool collision = 0;
  int cy, cx;
  int half_h, half_w;
  int r_size_y, r_size_x;
  const int border = 1;
  int old_cy, old_cx;

  int rooms = 0;
  const int max_rooms = rand() % 7 + 11;

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      if (y == 0 || y == rows - 1 || x == 0 || x == cols - 1) {
        map[y][x] = '%';
      } else {
        map[y][x] = '#';
      }
    }
  }

  while (rooms < max_rooms) {

    r_size_y = rand() % 5 + 8;
    r_size_x = rand() % 5 + 8;
    half_h = r_size_y / 2;
    half_w = r_size_x / 2;

    cy = rand() % (rows - 2 * half_h - 2 * border) + half_h + border;
    cx = rand() % (cols - 2 * half_w - 2 * border) + half_w + border;

    for (int y = cy - half_h; y <= cy + half_h; y++) {
      for (int x = cx - half_w; x <= cx + half_w; x++) {
        map[y][x] = ' ';
      }
    }

    rooms++;

    if (rooms >= 2) {
      int x = old_cx;
      int y = old_cy;

      // горизонталь до новой комнаты
      while (x != cx) {
        if (x < cx)
          x++;
        else
          x--;
        map[y][x] = ' ';
      }

      // вертикаль до новой комнаты
      while (y != cy) {
        if (y < cy)
          y++;
        else
          y--;
        map[y][x] = ' ';
      }
    }

    old_cy = cy;
    old_cx = cx;
  }
  /* for (int y = 0; y < map.size(); y++) {
      for (int x = 0; x < map[y].size(); x++) {
          map[y][x] = '#';
      }
  }
  for (int y = map.size() / 4; y < (map.size() - (map.size() / 4)); y++) {
      for (int x = map[y].size() / 4; x < (map[y].size() - (map[y].size() /
  4)); x++) { map[y][x] = ' ';
      }
  } */
}
