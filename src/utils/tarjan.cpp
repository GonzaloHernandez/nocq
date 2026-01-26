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
#include "tarjan.h"
#include <vector>

TarjanSCC::TarjanSCC(Game& g,GameView& view) 
:   g(g), view(view),
    indices(g.nvertices,-1), lowlink(g.nvertices,-1), onstack(g.nvertices,false) 
{        
}

//-----------------------------------------------------------------------------

std::vector<std::vector<int>> TarjanSCC::solveRAW() {
    for (int v=0; v<g.nvertices; v++) {
        if (indices[v] ==-1) {
            searchRAW(v);
        }
    }
    return sccs;
}

//-----------------------------------------------------------------------------

void TarjanSCC::searchRAW(int v) {
    indices[v] = lowlink[v] = index;
    index++;
    stack.emplace_back(v);
    onstack[v] = true;

    for(auto& e : g.outs[v]) {
        int w = g.targets[e];
        if (indices[w] == -1) {
            searchRAW(w);
            lowlink[v] = std::min(lowlink[v], lowlink[w]);
        }
        else if (onstack[w]) {
            lowlink[v] = std::min(lowlink[v], lowlink[w]);
        }
    }
    if (lowlink[v] == indices[v]) {
        std::vector<int> scc;
        while (true){
            int w = stack.back();
            stack.pop_back();
            onstack[w] = false;
            scc.push_back(w);
            if (w==v) break;
        }
        sccs.push_back(scc);            
    }
}

//-----------------------------------------------------------------------------

std::vector<std::vector<int>> TarjanSCC::solve() {
    for (auto& v : view.getVertices()) {
        if (indices[v] ==-1) {
            search(v);
        }
    }
    return sccs;
}

//-----------------------------------------------------------------------------

void TarjanSCC::search(int v) {
    indices[v] = lowlink[v] = index;
    index++;
    stack.emplace_back(v);
    onstack[v] = true;

    for(auto& e : view.getOuts(v)) {
        int w = g.targets[e];
        if (indices[w] == -1) {
            search(w);
            lowlink[v] = lowlink[v]<lowlink[w]?lowlink[v]:lowlink[w];
        }
        else if (onstack[w]) {
            lowlink[v] = lowlink[v]<lowlink[w]?lowlink[v]:lowlink[w];
        }
    }
    if (lowlink[v] == indices[v]) {
        std::vector<int> scc;
        while (true){
            int w = stack.back();
            stack.pop_back();
            onstack[w] = false;
            scc.push_back(w);
            if (w==v) break;
        }
        sccs.push_back(scc);            
    }
}
