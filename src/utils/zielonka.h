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
 * conditions). This file is based on the implementation of the paper "A Matrix-
 * Based Approach to Parity Games".
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License,
 * v. 2.0. If a copy of the MPL was not distributed with this file, You can get
 * one at https://mozilla.org/MPL/2.0/.
 * 
 *-----------------------------------------------------------------------------
 */
#ifndef ZIELONKA_H
#define ZIELONKA_H

#ifndef GAME_H
#include "game.h"
#endif

#include "iostream"

class Zielonka {
private:
    Game& g;
public:
    Zielonka(Game& g);

    std::vector<int> getBestVertices(bool* removed);
    void attractor(int player, std::vector<int>&U, bool* removed);
    std::array<std::vector<int>,2> search(bool* removed, int level=0);
    std::array<std::vector<int>,2> solve();
};

#endif // ZIELONKA_H
