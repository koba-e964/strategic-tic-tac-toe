#include <cassert>
#include <iostream>
#include <vector>
using namespace std;

typedef long long ll;
typedef ll Board[3]; // each cell occupies 2 bits.

// pos: 0~8, xy: 0~8
enum State {
  Vacant = 0,
  Black = 1,
  White = 2,
};

State get(Board b, int pos, int xy) {
  return (State)((b[pos / 3] >> (2 * ((pos % 3) * 9 + xy))) & 3);
}

void put(Board b, int pos, int xy, State s) {
  int shift = 2 * ((pos % 3) * 9 + xy);
  b[pos / 3] &= ~(3LL << shift);
  b[pos / 3] |= (ll)s << shift;
}

/*
 * 012
 * 345
 * 678
 */
int is_winning_ninebits(int nine) {
  return
    (nine & 0007) == 0007 ||
    (nine & 0070) == 0070 ||
    (nine & 0700) == 0700 ||
    (nine & 0111) == 0111 ||
    (nine & 0222) == 0222 ||
    (nine & 0444) == 0444 ||
    (nine & 0124) == 0124 ||
    (nine & 0421) == 0421;
}

void print_board(Board b) {
  for (int i = 0; i < 9; ++i) {
    if (i % 3 == 0 && i > 0) {
      cout << "-----------" << endl;
    }
    for (int j = 0; j < 9; ++j) {
      if (j % 3 == 0 && j > 0) {
	cout << "|";
      }
      State s = get(b, 3 * (i / 3) + j / 3, 3 * (i % 3) + (j % 3));
      cout << (s == 0 ? " " : s == 1 ? "x" : "o");
    }
    cout << endl;
  }
}
/*
 * sub is 18-bit long.
 */
bool is_winning_subboard(int sub, State col) {
  int set = 0;
  for (int i = 0; i < 9; ++i) {
    set |= (sub >> (2 * i) & 3) == col ? 1 << i : 0; 
  }
  return is_winning_ninebits(set);
}

bool is_finishing_subboard(int sub) {
  int filled = 0;
  for (int i = 0; i < 9; ++i) {
    filled |= (sub >> (2 * i)) & 3 ? 1 << i : 0;
  }
  return is_winning_subboard(sub, Black) || is_winning_subboard(sub, White) || filled == 0777;
}
/*
 * returned value:
 *   (-1, ??): error
 *   (0, obligation): obligation is a set of positions to place in next turn.
 */
pair<int, int> place_intelligent(Board b, int pos, int xy, State s) {
  int comp_status = 0;
  for (int i = 0; i < 9; ++i) {
    int subboard = b[i / 3] >> (18 * (i % 3));
    if (is_finishing_subboard(subboard)) {
      comp_status |= 1 << i;
    }
  }
  if (comp_status & (1 << pos)) {
    return pair<int, int>(-1, 0);
  }
  if (get(b, pos, xy) != Vacant) {
    return pair<int, int>(-1, 1);
  }
  put(b, pos, xy, s);
  // recompute comp_status
  for (int i = 0; i < 9; ++i) {
    int subboard = b[i / 3] >> (18 * (i % 3));
    if (is_finishing_subboard(subboard)) {
      comp_status |= 1 << i;
    }
  }
  int obligation = comp_status & (1 << xy) ? (0777 - comp_status) : (1 << xy);
  return pair<int, int>(0, obligation);
}

vector<int> placeability(Board b) {
  int comp_status = 0;
  vector<int> result(9, 0);
  for (int i = 0; i < 9; ++i) {
    int subboard = b[i / 3] >> (18 * (i % 3));
    if (is_finishing_subboard(subboard)) {
      comp_status |= 1 << i;
    }
  }
  for (int pos = 0; pos < 9; ++pos) {
    if (comp_status & (1 << pos)) {
      continue;
    }
    for (int xy = 0; xy < 9; ++xy) {
      result[pos] |= get(b, pos, xy) == Vacant ? 1 << xy : 0;
    }
  }
  return result;
}

bool is_winning_board(Board b, State col) {
  int comp_status = 0;
  for (int i = 0; i < 9; ++i) {
    if (is_winning_subboard(b[i / 3] >> (18 * (i % 3)), col)) {
      comp_status |= 1 << i;
    }
  }
  return is_winning_ninebits(comp_status);
}

int is_finishing_board(Board b) {
  bool bl = is_winning_board(b, Black);
  bool wh = is_winning_board(b, White);
  vector<int> pl = placeability(b);
  bool smother = true;
  for (int i = 0; i < 9; ++i) {
    if (pl[i] != 0) smother = false;
  }
  if (smother) {
    return 3;
  }
  if (bl && wh) {
    assert (!"black and white both won! What happened!?");
  }
  if (!bl && !wh) {
    return Vacant;
  }
  return bl ? Black : White;
}

int main(void) {
  assert (is_winning_ninebits(0323));
  Board b = {};
  State turn = Black;
  int obligation = 0777;
  srand(time(0));
  while (1) {
    print_board(b);
    int winner = is_finishing_board(b);
    if (winner != Vacant) {
      cout << "Game finished." << endl;
      if (winner == 3) {
	cout << "Draw" << endl;
      } else {
	cout << (winner == Black ? "x" : "o") << " won." << endl;
      }
      break;
    }
    int pos, xy;
    vector<int> pl = placeability(b);

    do {
      pos = rand() % 9;
      xy = rand() % 9;
    } while ((obligation & (1 << pos)) == 0 || (pl[pos] & (1 << xy)) == 0);

    cout << "pos=" << pos << ", xy=" << xy << endl;
    pair<int, int> result = place_intelligent(b, pos, xy, turn);
    if (result.first < 0) {
      cout << "invalid move" << endl;
      continue;
    }
    obligation = result.second;
    cout << "obligation = " << obligation << endl;
    turn = turn == Black ? White : Black;
  }
}
