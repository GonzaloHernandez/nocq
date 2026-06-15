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

TarjanSCC::TarjanSCC(Game& g, GameView& view) 
:   g(g), view(view) 
{        
    indices.growTo(g.nvertices, -1);
    lowlink.growTo(g.nvertices, -1);
    onstack.growTo(g.nvertices, false);
}

//-----------------------------------------------------------------------------

void TarjanSCC::solveRAW(vec<vec<int32_t>*>& out_sccs) {
    out_sccs.clear();
    index = 0;
    stack.clear();
    for (unsigned int v = 0; v < g.nvertices; v++) {
        indices[v] = -1;
        lowlink[v] = -1;
        onstack[v] = false;
    }

    for (unsigned int v = 0; v < g.nvertices; v++) {
        if (indices[v] == -1) {
            searchRAW(v, out_sccs);
        }
    }
}

//-----------------------------------------------------------------------------

void TarjanSCC::searchRAW(int32_t v, vec<vec<int32_t>*>& out_sccs) {
    indices[v] = lowlink[v] = index;
    index++;
    stack.push(v);
    onstack[v] = true;

    for (unsigned int i = 0; i < g.outs[v].size(); i++) {
        unsigned int e = g.outs[v][i];
        int32_t w = g.targets[e]; // Matches your vertex type
        if (indices[w] == -1) {
            searchRAW(w, out_sccs);
            lowlink[v] = std::min(lowlink[v], lowlink[w]);
        }
        else if (onstack[w]) {
            lowlink[v] = std::min(lowlink[v], indices[w]);
        }
    }

    if (lowlink[v] == indices[v]) {
        vec<int32_t>* scc = new vec<int32_t>();
        while (true) {
            int32_t w = stack.last();
            stack.pop();
            onstack[w] = false;
            scc->push(w);
            if (w == v) break;
        }
        out_sccs.push(scc);            
    }
}

//-----------------------------------------------------------------------------

void TarjanSCC::solve(vec<vec<int32_t>*>& out_sccs) {
    out_sccs.clear();
    index = 0;
    stack.clear();
    for (unsigned int v = 0; v < g.nvertices; v++) {
        indices[v] = -1;
        lowlink[v] = -1;
        onstack[v] = false;
    }

    vec<int32_t> vs;
    view.getVertices(vs);
    for (unsigned int i = 0; i < vs.size(); i++) {
        int32_t v = vs[i];
        if (indices[v] == -1) {
            search(v, out_sccs);
        }
    }
}

//-----------------------------------------------------------------------------

void TarjanSCC::search(int32_t v, vec<vec<int32_t>*>& out_sccs) {
    indices[v] = lowlink[v] = index;
    index++;
    stack.push(v);
    onstack[v] = true;

    vec<int32_t> es;
    view.getOuts(es, v);
    for (unsigned int i = 0; i < es.size(); i++) {
        int32_t e = es[i];
        int32_t w = g.targets[e]; // Matches your vertex type
        if (indices[w] == -1) {
            search(w, out_sccs);
            lowlink[v] = lowlink[v] < lowlink[w] ? lowlink[v] : lowlink[w];
        }
        else if (onstack[w]) {
            lowlink[v] = lowlink[v] < indices[w] ? lowlink[v] : indices[w];
        }
    }

    if (lowlink[v] == indices[v]) {
        vec<int32_t>* scc = new vec<int32_t>();
        while (true) {
            int32_t w = stack.last();
            stack.pop();
            onstack[w] = false;
            scc->push(w);
            if (w == v) break;
        }
        out_sccs.push(scc);            
    }
}