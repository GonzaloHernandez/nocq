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
#ifndef TARJAN_H
#define TARJAN_H

#ifndef GAME_H
#include "game.h"
#endif

#include <vector>

class TarjanSCC {
private:
    Game& g;
    GameView& view;
    std::vector<int>    indices;
    std::vector<int>    lowlink;
    std::vector<bool>   onstack;
    std::vector<int>     stack;
    std::vector<std::vector<int>>   sccs;
    int index = 0;
public:
    TarjanSCC(Game& g,GameView& view);
    
    std::vector<std::vector<int>> solveRAW();
    void searchRAW(int v);

    std::vector<std::vector<int>> solve();
    void search(int v);
};

#endif // TARJAN_H
