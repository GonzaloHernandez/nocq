#ifndef FRA_H
#define FRA_H

#include <iostream>
#include <vector>
#include <initializer_list>
#include <cstring>
#include <chrono>

#ifndef GAME_H 
#include "game.h"
#endif

// private functions:
// int findVertex(int vertex,std::vector<int>& path);
// int bestcolor(Game& g, int index,std::vector<int>& path);
// signed char getPlayBasic(Game& g, int p, std::vector<int> path, int current);
// signed char getPlayMemo(Game& g, int p, std::vector<int> path, int current, std::vector<int>& memo);

signed char getPlay(Game& g, int start, bool basic=false);
bool getAllCycles(Game& g, std::vector<int> path, int v, std::vector<bool>& touched);

#endif // FRA_H