/*
 * Main authors:
 *    Gonzalo Hernandez <gonzalo.hernandez@monash>
 *    <gonzalo.hernandez@udenar.edu.co>
 *
 * Contributing authors:
 *    Guido Tack <guido.tack@monash.edu>
 *    Julian Gutierrez <J.Gutierrez@sussex.ac.uk>
 *
 * This file is part of NOCQ (a CP Toolchain for parity games with quantitative
 * conditions).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License,
 * v. 2.0. If a copy of the MPL was not distributed with this file, You can get
 * one at https://mozilla.org/MPL/2.0/.
 * 
 *-----------------------------------------------------------------------------
 */
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
// signed char getPlayMemo(Game& g, int p, std::vector<int> path, int current, 
//                         std::vector<int>& memo);

signed char getPlay(Game& g, int init, bool basic=false);

bool getAllCycles(  Game& g, std::vector<int> path, int v, 
                    std::vector<bool>& touched);

#endif // FRA_H
