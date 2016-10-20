#include <iostream>
#include <algorithm>
#include <ctime>
#include <vector>
#include <cstdlib>
#include <iterator>
#include <windows.h>

// (c) 2016 HIRAISHIN SOFTWARE

using namespace std;

const int R_SIZE = 8, R_PCHAR = 32;
const unsigned char R_CHARS[] = {'_','p', 'J', 'S', 'V', 'Q', 'K'};
const int G_COST[] = {100, 200, 200, 400, 1000, 2000};
const int G_MVS[] = {8, 8, 28, 28, 56, 8};

enum Players {PLAYER, COMPUTER};
enum Types {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

/// MOVESETS
int moveset0[8][2] = {{0,1},{0,2},{1,1},{-1,1},{0,-2},{0,-1},{1,-1},{-1,-1}};
int moveset1[8][2] = {{1,2},{1,-2},{-1,2},{-1,-2},{2,1},{2,-1},{-2,1},{-2,-1}};
int moveset2[28][2] = {{1,1},{2,2},{3,3},{4,4},{5,5},{6,6},{7,7},{-1,-1},{-2,-2},{-3,-3},{-4,-4},{-5,-5},{-6,-6},{-7,-7},{-1,1},{-2,2},{-3,3},{-4,4},{-5,5},{-6,6},{-7,7},
    {1,-1},{2,-2},{3,-3},{4,-4},{5,-5},{6,-6},{7,-7}};
int moveset3[28][2] = {{-1,0},{-2,0},{-3,0},{-4,0},{-5,0},{-6,0},{-7,0},{1,0},{2,0},{3,0},{4,0},{5,0},{6,0},{7,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,-1},{0,-2},
    {0,-3},{0,-4},{0,-5},{0,-6},{0,-7}};
int moveset4[56][2] = {{-1,0},{-2,0},{-3,0},{-4,0},{-5,0},{-6,0},{-7,0},{1,0},{2,0},{3,0},{4,0},{5,0},{6,0},{7,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,-1},{0,-2},
    {0,-3},{0,-4},{0,-5},{0,-6},{0,-7},{1,1},{2,2},{3,3},{4,4},{5,5},{6,6},{7,7},{-1,-1},{-2,-2},{-3,-3},{-4,-4},{-5,-5},{-6,-6},{-7,-7},{-1,1},{-2,2},{-3,3},{-4,4},
    {-5,5},{-6,6},{-7,7},{1,-1},{2,-2},{3,-3},{4,-4},{5,-5},{6,-6},{7,-7}};
int moveset5[8][2] = {{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}};

struct Figure {
    Players player;
    Types type;
    int x, y;
    bool alive, defpos;
};

/// 0 OWNER, 1 X, 2 Y, 3 TYPE, 4 LIFE, 5 DEFAULT?
Figure game_data[R_PCHAR], predict_data[R_PCHAR];
unsigned char render_sheet [R_SIZE * 2][R_SIZE];

int steps = 0, win = 0, cycles;

int victory(void);

/// FINAL
void g_ply(void);
void g_com(void);
void g_setup(Players player);
void r_render(bool debug, Figure data[]);
void g_copy(Figure data[], Figure data_copy[]);
bool g_isPresent(Figure data[], int id);
bool g_moveTo(Figure data[], int id, int x2, int y2, Players player);
bool g_poss(Figure data[], int id, int x2, int y2, bool ignore_owner);
int g_getInt(Types type);
int g_getScore(Types type);
int g_at(Figure data[], int x, int y);
int g_arePresent(Figure data[], Players player);
char g_getChar(Types type);
vector <int> g_areTargets(Figure data[], Players player, int index);
vector <int> g_areDangerous(Figure data[], Players player, int exclude);
vector <int> g_areDangers(Figure data[], Players player, int x, int y, bool ignore_owner);

int main()
{
    srand((unsigned int) time(NULL));
    g_setup(PLAYER);
    do {
        r_render(false, game_data);
        win = victory();
        if (win != 0) break;
        g_ply();
        steps++;
        r_render(false, game_data);
        win = victory();
        if (win != 0) break;
        g_com();
    } while (win == 0);
    r_render(false, game_data);
    if (win == 1) cout << endl << "PLAYER 1 WON !" << endl << "It took you " << steps << " to win ...";
    else if (win == 2) cout << endl << "PLAYER COM WON!" << endl << "You got fucking REKT mate ...";
    else if (win == 3) cout << endl << "You tied with AI, good fucking game you fucking looser ...";
    return 0;
}

int victory(void) {
    if (g_arePresent(game_data, PLAYER) == 1 && g_arePresent(game_data, COMPUTER) == 1) return 3;
    else if (!game_data[31].alive) return 1;
    else if (!game_data[15].alive) return 2;
    return 0;
}

/// UPDATED:
void g_com(void) {
    cycles = 0;
    vector <int> rand_pieces;
    int step [R_PCHAR / 2][4]; // SCORE, GOTO x, GOTO y
    cout << "Waiting for COM turn ..." << endl;

    rand_pieces.clear();
    for (int i = 16; i < R_PCHAR; i++) {
        if (game_data[i].alive && i != R_PCHAR - 1) rand_pieces.push_back(i);
        for (int j = 0; j < 4; j++) step[i - R_PCHAR / 2][j] = (j == 0 ? -2001 : (j == 3 ? i : -1));
    }
    random_shuffle(rand_pieces.begin(), rand_pieces.end());
    rand_pieces.push_back(31);

    for (int n = 0; (unsigned int) n < rand_pieces.size(); n++) {
        int i = rand_pieces[n];
        if (!game_data[i].alive) continue;
        g_copy(game_data, predict_data);
        cout << "Vect#" << n << " contains item #" << i << " [" << game_data[i].y << "," << game_data[i].x << "]" << endl; ///DEBUG

        Types i_type = game_data[i].type;
        int (*moves)[2] = (i_type == PAWN ? moveset0 : (i_type == KNIGHT ? moveset1 : (i_type == BISHOP ? moveset2 : (i_type == ROOK ? moveset3 : (i_type == QUEEN ? moveset4 : moveset5)))));

        for (int i_move_iter = 0; i_move_iter < G_MVS[g_getInt(i_type)]; i_move_iter++) {cycles++;
            int it = game_data[i].x + moves[i_move_iter][0];
            int yt = game_data[i].y + moves[i_move_iter][1];
            if (it < 0 || it >= R_SIZE || yt < 0 || yt >= R_SIZE) continue;
            if (!g_poss(game_data, i, it, yt, false)) continue;
            int turn_score = -2002;
            cout << "> Available move: [" << yt << "," << it << "] ";

            int danger_aim = g_at(game_data, it, yt);
            vector<int> danger_stay = g_areDangers(game_data, PLAYER, game_data[i].x, game_data[i].y, false);

            /// WIP BEGIN

            if (g_at(predict_data, it, yt) != -1) {
                g_copy(game_data, predict_data);
                predict_data[g_at(predict_data, it, yt)].alive = false;
            }
            predict_data[i].x = it;
            predict_data[i].y = yt;
            vector<int> danger_move = g_areDangers(predict_data, PLAYER, it, yt, false);
            vector<int> danger_aim_predict = g_areTargets(predict_data, COMPUTER, i);
            vector<int> danger_move_begin = g_areDangerous(game_data, COMPUTER, i);
            vector<int> danger_move_mask = g_areDangerous(predict_data, COMPUTER, i);
            vector<int> danger_move_end;

            set_difference(danger_move_begin.begin(), danger_move_begin.end(), danger_move_mask.begin(), danger_move_mask.end(), back_inserter(danger_move_mask));

            /// WIP END

            if (danger_aim != -1) cout << "-WKILL "; ///DEBUG
            if (danger_move.size() > 0) cout << "-MVDIE "; ///DEBUG
            if (danger_stay.size() > 0) cout << "-STDIE "; ///DEBUG
            if (danger_aim_predict.size() > 0 && danger_move.size() == 0) cout << "-PEKIL "; ///DEBUG
            if (danger_move_end.size() > 0) cout << "-MVDMG"; ///DEBUG

            if (danger_aim != -1) turn_score = g_getScore(game_data[danger_aim].type);
            else turn_score = -g_getScore(game_data[i].type) / 100;
            if (danger_stay.size() > 0) turn_score += g_getScore(game_data[i].type);
            if (danger_move.size() > 0) turn_score -= g_getScore(game_data[i].type);
            if (danger_aim_predict.size() > 0 && danger_move.size() == 0) turn_score += g_getScore(predict_data[danger_aim_predict[0]].type) / 10;
            if (danger_move_end.size() > 0) turn_score -= g_getScore(predict_data[danger_move_end[0]].type);

            cout << endl << "  Move gain: " << turn_score; ///DEBUG
            if (turn_score > step[i - R_PCHAR / 2][0]) {
                    cout << " / Gain record: " << turn_score;
                    step[i - R_PCHAR / 2][0] = turn_score;
                    step[i - R_PCHAR / 2][1] = it;
                    step[i - R_PCHAR / 2][2] = yt;
                    step[i - R_PCHAR / 2][3] = i;
            }
            cout << endl; ///DEBUG
        }
    }

    cout << endl << "Movelist:" << endl; ///DEBUG
    for (int i = 0; i < R_PCHAR / 2; i++) {
        if (step[i][0] == -2001) continue;
        cout << "> Move " << (i < 10 ? " " : "") << i << " [" << step[i][2]<< "," << step[i][1] << "] " << step[i][0] << endl; ///DEBUG
    }

    vector<int*> choice;
    int choice_score = step[0][0], chosen_id;
    for (int i = 1; i < R_PCHAR / 2; i++) if(step[i][0] > choice_score) choice_score = step[i][0];
    for (int i = 0; i < R_PCHAR / 2; i++) if(step[i][0] == choice_score) choice.push_back(step[i]);
    random_shuffle(choice.begin(), choice.end());
    chosen_id = choice[0][3];

    cout << "Move with maximum gain (took " << cycles << " cycles):"<< endl; ///DEBUG
    cout << choice[0][0] << " with piece " << choice[0][3] << " >> [" << choice[0][2] << "," << choice[0][1] << "]" << endl; ///DEBUG
    if (!g_moveTo(game_data, chosen_id, choice[0][1], choice[0][2], COMPUTER)) game_data[31].alive = false;

    system("pause>nul");
}

int g_getScore(Types type) {
    if (type == PAWN) return G_COST[0];
    else if (type == KNIGHT) return G_COST[1];
    else if (type == BISHOP) return G_COST[2];
    else if (type == ROOK) return G_COST[3];
    else if (type == QUEEN) return G_COST[4];
    else if (type == KING) return G_COST[5];
    else return -9999;
}

char g_getChar(Types type) {
    if (type == PAWN) return R_CHARS[1];
    else if (type == KNIGHT) return R_CHARS[2];
    else if (type == BISHOP) return R_CHARS[3];
    else if (type == ROOK) return R_CHARS[4];
    else if (type == QUEEN) return R_CHARS[5];
    else if (type == KING) return R_CHARS[6];
    else return R_CHARS[0];
}

int g_getInt(Types type) {
    if (type == PAWN) return 0;
    else if (type == KNIGHT) return 1;
    else if (type == BISHOP) return 2;
    else if (type == ROOK) return 3;
    else if (type == QUEEN) return 4;
    else if (type == KING) return 5;
    else return -1;
}

// https://www.youtube.com/watch?v=FtT2r8pFUj4
bool g_poss(Figure data[], int id, int x2, int y2, bool ignore_owner) {
	if (x2 < 0 || x2 > R_SIZE || y2 < 0 || y2 > R_SIZE) return false;
    else if (!g_isPresent(data, id)) return false;

    Types type = data[id].type;
    Players player = data[id].player;
    int x = data[id].x;
    int y = data[id].y;
    bool def = data[id].defpos;

    if (type == PAWN) {
        if (x2 == x && g_at(data, x2, y2) == -1) {
            if (y2 - y == (player == PLAYER ? 1 : -1)) return true;
            else if (y2 - y == (player == PLAYER ? 2 : -2) && g_at(data, x, y + (y2 > y ? 1 : -1)) == -1 && def) return true;
            else return false;
        } else if ((x2 - x)*(x2 - x) == 1 && g_at(data, x2, y2) != -1) {
            if (y2 - y == (player == PLAYER ? 1 : -1)) {
                if (!ignore_owner && data[g_at(data, x2, y2)].player == player) return false;
                else return true;
            } else return false;
        } else return false;
    } else if (type == KNIGHT) {
        if ((x2 - x)*(x2 - x) + (y2 - y)*(y2 - y) == 5) {
            if (g_at(data, x2, y2) != -1) {
                if (!ignore_owner && data[g_at(data, x2, y2)].player == player) return false;
                else return true;
            } else return true;
        } else return false;
    } else if (type == BISHOP) {
        if (x2 - x == y2 - y || x2 - x == y - y2) {
            int s_x = (x2 > x ? 1 : -1), s_y = (y2 > y ? 1 : -1);
            for (int s_i = 1; (x2 > x ? x + s_i * s_x < x2 : x + s_i * s_x > x2) ; s_i++) {
                if (g_at(data, x + s_i * s_x, y + s_i * s_y) != -1) return false;
            }
            if (g_at(data, x2, y2) != -1) {
                if (!ignore_owner && data[g_at(data, x2, y2)].player == player) return false;
                else return true;
            } else return true;
        } else return false;
    } else if (type == ROOK) {
        if (x2 == x) {
            for (int s_y = (y2 > y) ? y + 1 : y - 1; (y2 > y ? s_y < y2 : s_y > y2); s_y += (y2 > y) ? 1 : -1) {
                if (g_at(data, x2, s_y) != -1) return false;
            }
        } else if (y2 == y) {
            for (int s_x = (x2 > x ? x + 1 : x - 1); (x2 > x ? s_x < x2 : s_x > x2); s_x += (x2 > x ? 1 : -1)) {
                if (g_at(data, s_x, y2) != -1) return false;
            }
        } else return false;
        if (g_at(data, x2, y2) != -1) {
            if (!ignore_owner && data[g_at(data, x2, y2)].player == player) return false;
            else return true;
        } else return true;
    } else if (type == QUEEN) {
        if (x2 == x || y2 == y) {
            if (x2 == x) {
                for (int s_y = (y2 > y ? y + 1 : y - 1); (y2 > y ? s_y < y2 : s_y > y2); s_y += (y2 > y ? 1 : -1)) {
                    if (g_at(data, x2, s_y) != -1) return false;
                }
            } else if (y2 == y) {
                for (int s_x = (x2 > x ? x + 1 : x - 1); (x2 > x ? s_x < x2 : s_x > x2); s_x += (x2 > x ? 1 : -1)) {
                    if (g_at(data, s_x, y2) != -1) return false;
                }
            } else return false;
            if (g_at(data, x2, y2) != -1) {
                if (!ignore_owner && data[g_at(data, x2, y2)].player == player) return false;
                else return true;
            } else return true;
        } else if (x2 - x == y2 - y || x2 - x == y - y2) {
            int s_x = (x2 > x ? 1 : -1), s_y = (y2 > y ? 1 : -1);
            for (int s_i = 1; (x2 > x ? x + s_i * s_x < x2 : x + s_i * s_x > x2) ; s_i++) {
                if (g_at(data, x + s_i * s_x, y + s_i * s_y) != -1) return false;
            }
            if (g_at(data, x2, y2) != -1) {
                if (!ignore_owner && data[g_at(data, x2, y2)].player == player) return false;
                else return true;
            } else return true;
        } else return false;
    } else if (type == KING) {
        if (x2 - x > -2 && x2 - x < 2 && y2 - y > -2 && y2 - y < 2) {
            for (int s_i = (player == PLAYER ? R_PCHAR / 2 : 0); s_i < (player == PLAYER ? R_PCHAR : R_PCHAR / 2); s_i++) {
                if (g_isPresent(data, s_i) && g_poss(data, s_i, x2, y2, false)) return false;
                else continue;
            }
            if (g_at(data, x2, y2) != -1) {
                if (!ignore_owner && data[g_at(data, x2, y2)].player == player) return false;
                else return true;
            } else return true;
        } else return false;
    } else return false;
}

void r_render(bool debug, Figure data[]) {
    if (!debug) {
        system("cls"); /// SECURITY ISSUE ///
        for (int i = 0; i < 2 * R_SIZE * R_SIZE; render_sheet[i % (2 * R_SIZE)][i / (2 * R_SIZE)] = R_CHARS[0], i++);
        for (int i = 0; i < R_PCHAR; i++) {
            if (data[i].alive) {
                render_sheet[data[i].x * 2 + 1][data[i].y] = g_getChar(data[i].type);
                render_sheet[data[i].x * 2][data[i].y] = (data[i].player == PLAYER ? ' ' : g_getChar(data[i].type));
            }
        }
        for (int i = R_SIZE - 1; i >= 0; i--) {
            cout << i << " >";
            for (int y = 0; y < R_SIZE; y++) {
                cout << " " << render_sheet[y * 2][i] << render_sheet[y * 2 + 1][i] << " ";
            }
            cout << endl << endl;
        }
        cout << "   ";
        for (int i = 0; i < R_SIZE; i++) cout << "  " << i << " ";
        cout << endl << endl;
    } else {
        for (unsigned int i = 0; i < (sizeof(R_CHARS)/sizeof(*R_CHARS)); i++) {
            cout << R_CHARS[i] << endl;
        }
    }
}

int g_at(Figure data[], int x, int y) {
    for(int i = 0; i < R_PCHAR; i++) {
        if (data[i].x == x && data[i].y == y && data[i].alive) return i;
    }
    return -1;
}

int g_arePresent(Figure data[], Players player) {
    int alive = 0;
    for (int i = (player == PLAYER ? 0 : R_PCHAR / 2); i < (player == PLAYER ? R_PCHAR / 2 : R_PCHAR); i++) {
        if (data[i].alive) alive++;
    }
    return alive;
}

bool g_isPresent(Figure data[], int id) {
    return (data[id].alive);
}

void g_copy(Figure data[], Figure data_copy[]) {
    for (int i = 0; i < R_PCHAR; i++) {
        for (int y = 0; y < 6; y++) data_copy[i] = data[i];
    }
}

bool g_moveTo(Figure data[], int id, int x2, int y2, Players player) {
    if (player == data[id].player && g_poss(data, id, x2, y2, false)) {
        if (g_at(data, x2, y2) != -1) data[g_at(data, x2, y2)].alive = false;
        data[id].x = x2;
        data[id].y = y2;
        if ((y2 == 0 || y2 == R_SIZE - 1) && data[id].type == PAWN) data[id].type = QUEEN;
        data[id].defpos = false;
        return true;
    } else return false;
}

vector <int> g_areDangers(Figure data[], Players player, int x, int y, bool ignore_owner) {
    vector <int> can;
    for (int i = (player == PLAYER ? 0 : 16); i < (player == PLAYER? 16 : 32); i++) if (g_poss(data, i, x, y, ignore_owner)) can.push_back(i);
    return can;
}

vector <int> g_areDangerous(Figure data[], Players player, int exclude) {
    vector <int> endangered;
    for (int i = (player == PLAYER ? 0 : R_PCHAR / 2); i < (player == PLAYER ? R_PCHAR / 2 : R_PCHAR); i++) if (i != exclude && g_areDangers(data, (player == PLAYER ? COMPUTER : PLAYER), data[i].x, data[i].y, false).size() > 0) endangered.push_back(i);
    return endangered;
}

vector <int> g_areTargets(Figure data[], Players player, int index) {
    vector <int> targets;
    for (int i = 0; i < R_PCHAR / 2; i++) if (g_poss(data, index, data[(player == COMPUTER ? i : i + 16)].x, data[(player == COMPUTER ? i : i + 16)].y, false)) targets.push_back((player == COMPUTER ? i : i + 16));
    return targets;
}

void g_setup(Players player) {
    int side = (player == PLAYER ? 0 : 1);
    for (int i = 0; i < 8; i++) {
        game_data[i + side * 16].player = player;
        game_data[i + side * 16].x = i;
        game_data[i + side * 16].y = 1 + side * 5;
        game_data[i + side * 16].type = PAWN;
        game_data[i + side * 16].alive = true;
        game_data[i + side * 16].defpos = true;
    }
    for (int i = 8; i < 10; i++) {
        game_data[i + side * 16].player = player;
        game_data[i + side * 16].x = (i == 8) ? 0 : 7;
        game_data[i + side * 16].y = side * 7;
        game_data[i + side * 16].type = ROOK;
        game_data[i + side * 16].alive = true;
        game_data[i + side * 16].defpos = true;
    }
    for (int i = 10; i < 12; i++) {
        game_data[i + side * 16].player = player;
        game_data[i + side * 16].x = (i == 10) ? 1 : 6;
        game_data[i + side * 16].y = side * 7;
        game_data[i + side * 16].type = KNIGHT;
        game_data[i + side * 16].alive = true;
        game_data[i + side * 16].defpos = true;
    }
    for (int i = 12; i < 14; i++) {
        game_data[i + side * 16].player = player;
        game_data[i + side * 16].x = (i == 12) ? 2 : 5;
        game_data[i + side * 16].y = side * 7;
        game_data[i + side * 16].type = BISHOP;
        game_data[i + side * 16].alive = true;
        game_data[i + side * 16].defpos = true;
    }

    game_data[14 + side * 16].player = player;
    game_data[14 + side * 16].x = 3;
    game_data[14 + side * 16].y = side * 7;
    game_data[14 + side * 16].type = QUEEN;
    game_data[14 + side * 16].alive = true;
    game_data[14 + side * 16].defpos = true;

    game_data[15 + side * 16].player = player;
    game_data[15 + side * 16].x = 4;
    game_data[15 + side * 16].y = side * 7;
    game_data[15 + side * 16].type = KING;
    game_data[15 + side * 16].alive = true;
    game_data[15 + side * 16].defpos = true;

    if (player == PLAYER) g_setup(COMPUTER);
}

void g_ply(void) {
    int x, y, x2, y2;
    do {
        cout << "Your turn [ROW, COL]: ";
        cin >> y >> x >> y2 >> x2;
    } while (!g_moveTo(game_data, g_at(game_data, x, y), x2, y2, PLAYER));
}
