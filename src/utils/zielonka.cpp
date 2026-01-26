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
#include "zielonka.h"
#include "array"

Zielonka::Zielonka(Game& g) : g(g) {
}

//-----------------------------------------------------------------------------

std::vector<int> Zielonka::getBestVertices(bool* removed) {
    std::vector<int> bestVertices;
    bool found = false;
    int bestColor;
    for (int i=0; i<g.nvertices; i++) {
        if (removed[i]) continue;

        if (!found) {
            bestColor = g.priors[i];
            bestVertices.push_back(i);
            found = true;
            continue;
        }
        
        if (g.priors[i] == bestColor) {
            bestVertices.push_back(i);
        }
        else if (g.reward==MIN && g.priors[i] < bestColor) {
            bestColor = g.priors[i];
            bestVertices.clear();
            bestVertices.push_back(i);
        }
        else if (g.reward==MAX && g.priors[i] > bestColor) {
            bestColor = g.priors[i];
            bestVertices.clear();
            bestVertices.push_back(i);
        }
    }
    return bestVertices;
}

//-----------------------------------------------------------------------------

void Zielonka::attractor(int player, std::vector<int>&U, bool* removed) {
    std::unique_ptr<int[]> d = std::make_unique<int[]>(g.nvertices);
    std::fill_n(d.get(), g.nvertices, 0ull);

    for(auto& w : U) d[w] = 1ull;
    for(int i=0ull; i<U.size(); i++) {
        int w = U[i];
        for(auto& e : g.ins[w]) {
            int v = g.sources[e];
            if (removed[v]) continue;
            bool ally = g.owners[v] == player;
            if (d[v] == 0) {
                if (ally) {
                    U.push_back(v);
                    d[v] = 1;
                }
                else {
                    int outbound = 0ull;
                    for(auto& e_ : g.outs[v]) {
                        if (!removed[g.targets[e_]]) outbound++;
                    }
                    d[v] = outbound;
                    if (outbound == 1) U.push_back(v);
                }
            }
            else if (!ally && d[v] > 1) {
                d[v] -= 1ull;
                if (d[v] == 1) U.push_back(v);
            }
        }
    }
    for (auto& w : U) {
        removed[w] = true;
    }
}

//-----------------------------------------------------------------------------

std::array<std::vector<int>,2> Zielonka::search(bool* removed, int level) {
    std::vector<int> A = getBestVertices(removed);
    if (A.size() == 0) {
        return { std::vector<int>(), std::vector<int>() };
    }
    int player = g.priors[A[0]] % 2;
    std::unique_ptr<bool[]> removed1 = std::make_unique<bool[]>(g.nvertices);
    std::copy_n(removed, g.nvertices, removed1.get());

    attractor(player, A, removed1.get());
    auto win1 = search(removed1.get(),level+1); 
    if (!win1[1-player].size()) {
        win1[player].reserve(win1[player].size() + A.size());
        win1[player].insert(win1[player].end(), A.begin(), A.end());
        return win1;
    }
    else {
        std::unique_ptr<bool[]> removed2 = std::make_unique<bool[]>(g.nvertices);
        std::copy_n(removed, g.nvertices, removed2.get());
        std::vector<int> B(win1[1-player]);
        attractor(1-player, B, removed2.get());
        auto win2 = search(removed2.get(),level+1);
        win2[1-player].reserve(win2[1-player].size() + B.size());
        win2[1-player].insert(win2[1-player].end(), B.begin(), B.end());
        return win2;
    }
}

//-----------------------------------------------------------------------------

std::array<std::vector<int>,2> Zielonka::solve() {
    std::unique_ptr<bool[]> removed = std::make_unique<bool[]>(g.nvertices);
    std::fill_n(removed.get(), g.nvertices, false);
    return search(removed.get());
}
