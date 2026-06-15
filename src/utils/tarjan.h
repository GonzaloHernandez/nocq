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

class TarjanSCC {
private:
    Game& g;
    GameView& view;
    vec<int32_t>  indices; 
    vec<int32_t>  lowlink;
    vec<bool>     onstack;
    vec<int32_t>  stack;
    int32_t index = 0;
public:
    TarjanSCC(Game& g, GameView& view);
    
    void solveRAW(vec<vec<int32_t>*>& out_sccs);
    void searchRAW(int32_t v, vec<vec<int32_t>*>& out_sccs);

    void solve(vec<vec<int32_t>*>& out_sccs);
    void search(int32_t v, vec<vec<int32_t>*>& out_sccs);
};

#endif // TARJAN_H
