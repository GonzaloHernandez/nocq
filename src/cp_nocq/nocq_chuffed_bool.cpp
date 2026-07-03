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
#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"
#include "initializer_list"

#ifndef WINNING_CONDITIONS_H
#include "winning_conditions.h"
#endif

#ifndef TARJAN_H
#include "utils/tarjan.h"
#endif

namespace ChuffedBool {

class NOCCheckerSCC : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    parity_type playerSAT;

public:
    //-----------------------------------------------------------------------
    NOCCheckerSCC(Game& g, vec<BoolView>& V, vec<BoolView>& E, parity_type playerSAT) 
    : g(g), V(V), E(E), playerSAT(playerSAT)
    {
        for (unsigned int i = 0; i < g.owners.size(); i++)  V[i].attach(this, 1, EVENT_F);
        for (unsigned int i = 0; i < g.sources.size(); i++) E[i].attach(this, 1, EVENT_F);
    }

    //-----------------------------------------------------------------------
    // Returns the best vertices by reference using vec<>
    void getBestColors(const vec<int32_t>& subgraph, vec<int32_t>& best_vertices) {
        best_vertices.clear();
        if (subgraph.size() == 0) return;

        best_vertices.push(subgraph[0]);

        for (unsigned int i = 1; i < subgraph.size(); i++) {
            int32_t v = subgraph[i];
            int32_t best = best_vertices[0];

            if (g.isBetter(g.priors[v], g.priors[best])) {
                best_vertices.clear();
                best_vertices.push(v);
            } else if (g.priors[v] == g.priors[best]) {
                best_vertices.push(v);
            }
        }
    }

    //-----------------------------------------------------------------------
    bool backtrack() 
    {
        vec<Lit> lits;
        lits.push(); // First slot reserved for explanation placeholder
        for (unsigned int i = 1; i < g.nvertices; i++)   lits.push(V[i].getValLit());
        for (unsigned int i = 0; i < g.nedges; i++)      lits.push(E[i].getValLit());
        Clause* reason = Reason_new(lits);
        V[0].setVal(V[0].isFalse(), reason);
        return false;
    }

    //-----------------------------------------------------------------------
    bool propagate() override {

        GameView view(g);

        for (unsigned int i = 0; i < g.nvertices; i++) {
            if (!V[i].isFixed()) return true;
            view.vs[i] = (V[i].isTrue());
        }
        for (unsigned int i = 0; i < g.nedges; i++) {
            if (!E[i].isFixed()) return true;
            view.es[i] = (E[i].isTrue());
        }

        vec<vec<int32_t>*> stack;

        TarjanSCC initial_tar(g, view);
        initial_tar.solve(stack);

        // Pre-allocate a lookup table for O(1) membership checks instead of unordered_set
        vec<bool> is_best_color;
        is_best_color.growTo(g.nvertices, false);

        while (stack.size() > 0) {
            vec<int32_t>* sc_ptr = stack.last();
            stack.pop();
            vec<int32_t>& sc = *sc_ptr;

            if (sc.size() == 1) {
                int32_t v = sc[0];
                for (unsigned int i = 0; i < g.outs[v].size(); i++) {
                    int32_t e = g.outs[v][i];
                    if (E[e].isFalse()) continue;
                    int32_t w = g.targets[e];
                    if (v == w && g.priors[v] % 2 == opponent(playerSAT)) {
                        delete sc_ptr;
                        for (unsigned int k = 0; k < stack.size(); k++) delete stack[k];
                        return backtrack();
                    }
                }
                delete sc_ptr;
                continue;
            }

            // Find best colors natively using vec
            vec<int32_t> bestVertices;
            getBestColors(sc, bestVertices);

            int32_t v0 = bestVertices[0];
            if (g.priors[v0] % 2 == opponent(playerSAT)) {
                delete sc_ptr;
                for (unsigned int k = 0; k < stack.size(); k++) delete stack[k];
                return backtrack();
            }

            // Mark membership on our fast lookup table
            for (unsigned int i = 0; i < bestVertices.size(); i++) {
                is_best_color[bestVertices[i]] = true;
            }

            // Sub-filtering processing phase
            view.deactiveAll();
            for (unsigned int i = 0; i < sc.size(); i++) {
                int32_t v = sc[i];
                if (!is_best_color[v]) {
                    view.vs[v] = true;
                }
            }
            for (unsigned int i = 0; i < sc.size(); i++) {
                int32_t v = sc[i];
                if (!is_best_color[v]) {
                    for (unsigned int j = 0; j < g.outs[v].size(); j++) { 
                        int32_t e = g.outs[v][j];
                        int32_t w = g.targets[e];
                        if (E[e].isTrue() && view.vs[w]) {
                            view.es[e] = true;
                        }
                    }
                }
            }

            // Clear lookup flags back to false for the next iterations
            for (unsigned int i = 0; i < bestVertices.size(); i++) {
                is_best_color[bestVertices[i]] = false;
            }

            // Run sub-decomposition on the updated graph view
            TarjanSCC sub_tar(g, view);
            vec<vec<int32_t>*> sub_sccs;
            sub_tar.solve(sub_sccs);

            for (unsigned int i = 0; i < sub_sccs.size(); i++) {
                stack.push(sub_sccs[i]);
            }

            delete sc_ptr;
        }
        return true;
    }

    //-----------------------------------------------------------------------
    void wakeup(int i, int) override {
        pushInQueue();
    }

    //-----------------------------------------------------------------------
    void clearPropState() override {
        in_queue = false;
    }
};

//=============================================================================

class NOCCheckerBellmanFord : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    parity_type playerSAT;

public:
    //-----------------------------------------------------------------------
    NOCCheckerBellmanFord(Game& g, vec<BoolView>& V, vec<BoolView>& E, parity_type playerSAT) 
    : g(g), V(V), E(E), playerSAT(playerSAT)
    {
        for (unsigned int i = 0; i < g.owners.size(); i++)  V[i].attach(this, 1, EVENT_F);
        for (unsigned int i = 0; i < g.sources.size(); i++) E[i].attach(this, 1, EVENT_F);
    }

    //-----------------------------------------------------------------------
    bool backtrack() 
    {
        vec<Lit> lits;
        lits.push(); // First slot reserved for explanation placeholder
        for (unsigned int i = 1; i < g.nvertices; i++)   lits.push(V[i].getValLit());
        for (unsigned int i = 0; i < g.nedges; i++)      lits.push(E[i].getValLit());
        Clause* reason = Reason_new(lits);
        V[0].setVal(V[0].isFalse(), reason);
        return false;
    }

    //-----------------------------------------------------------------------
    bool propagate() override {
        GameView view(g);
        vec<int32_t>    vs;
        vec<int32_t>    es;

        for (unsigned int i = 0; i < g.nvertices; i++) {
            if (!V[i].isFixed()) return true;
            view.vs[i] = (V[i].isTrue());
            vs.push(i);
        }
        for (unsigned int i = 0; i < g.nedges; i++) {
            if (!E[i].isFixed()) return true;
            view.es[i] = (E[i].isTrue());
            es.push(i);
        }

        int32_t nvertices   = vs.size();
        int32_t nedges      = es.size();

        vec<int32_t> d(g.nvertices,0);
        vec<int32_t> pred(g.nvertices,-1);

        for (size_t i; i<nvertices-1; i++) {
            for (size_t j; j<nvertices; j++) { 
                int32_t u = vs[j];
                vec<int32_t> outs;
                view.getOuts(outs,u);
                for (size_t k=0; k<outs.size(); k++) {
                    int32_t e = outs[k];
                    int32_t v = g.targets[e];
                    if (d[u] + g.weights[e] < d[v]) {
                        d[v] = d[u] + g.weights[e];
                        pred[v] = u;
                    }
                }
            }
        }
        for (size_t j; j<nvertices; j++) { 
            int32_t u = vs[j];
            vec<int32_t> outs;
            view.getOuts(outs,u);
            for (size_t k=0; k<outs.size(); k++) {
                int32_t e = outs[k];
                int32_t v = g.targets[e];
                if (d[u] + g.weights[e] < d[v]) {
                    return backtrack();
                }
            }
        }

        return true;
    }

    //-----------------------------------------------------------------------
    void wakeup(int i, int) override {
        pushInQueue();
    }

    //-----------------------------------------------------------------------
    void clearPropState() override {
        in_queue = false;
    }
};

//=============================================================================

class NOCPropagator : public Propagator {
private:
    Game&                   g;
    vec<BoolView>           V;
    vec<BoolView>           E;
    parity_type             playerSAT;
    vec<WinningCondition*>  winConditions;

    const int   CF_STAY     = 1;
    const int   CF_CONFLICT = 2;
    const int   CF_DONE     = 3;

public:

    NOCPropagator(Game& g, vec<BoolView>& V, vec<BoolView>& E, 
        parity_type playerSAT, vec<WinningCondition*> winConditions)
    : g(g), V(V), E(E), playerSAT(playerSAT), winConditions(winConditions)
    {
        for (size_t i=0; i<g.nvertices;i++) V[i].attach(this, 1 , EVENT_F );
        for (size_t i=0; i<g.nedges;   i++) E[i].attach(this, 1 , EVENT_F );
    }

    //-------------------------------------------------------------------------
    
    int32_t findVertex(int32_t vertex,vec<int32_t>& path) {
        for (size_t i=0; i<path.size(); i++) {
            if (path[i] == vertex) return i;
        }
        return -1;
    }
    
    //-------------------------------------------------------------------------
    
    void clausify(vec<int32_t>& path, vec<BoolView> &B, vec<Lit>& lits) {
        for (size_t i=0; i<path.size()-1; i++) {
            lits.push(B[path[i]].getValLit());
        }
    }
    
    //-------------------------------------------------------------------------
    
    bool satisfiedConditions(   vec<int32_t>& pathV, vec<int32_t>& pathE,
                                vec<int64_t>& pathW, int32_t index) 
    {
        if (playerSAT==EVEN) {
            for (size_t i=0; i<winConditions.size(); i++) {
                if (!winConditions[i]->satisfy(pathV,pathE,pathW,index)) {
                    return false;
                }
            }
            return true;
        }
        else {
            for (size_t i=0; i<winConditions.size(); i++) {
                if (winConditions[i]->satisfy(pathV,pathE,pathW,index)) {
                    return true;
                }
            }
            return false;
        }
    }

    //-------------------------------------------------------------------------
    
    int filter(vec<int32_t>& pathV, vec<int32_t>& pathE, vec<int64_t>& pathW,
        int32_t v, int32_t lastEdge, bool definedEdge) 
    {
        int32_t index = findVertex(v,pathV);
        if (index >= 0) {

            if (!satisfiedConditions(pathV,pathE,pathW,index)) {
                vec<Lit> lits;
                lits.push();
                clausify(pathE,E,lits);
                Clause* reason = Reason_new(lits);
                if (! E[lastEdge].setVal(false,reason)) {
                    return CF_CONFLICT;
                }
            }
        }
        else if (definedEdge) {
            pathV.push(v);
            for (size_t i=0; i<g.outs[v].size(); i++) {
                int32_t e = g.outs[v][i];
                if (E[e].isFalse()) continue;

                int w = g.targets[e];
                int64_t acum = pathW.size() ? g.weights[e]+pathW.last()
                                            : g.weights[e];
                pathE.push(e);
                pathW.push(acum);
                int status = filter(pathV, pathE, pathW, w, e, E[e].isTrue());
                pathW.pop();
                pathE.pop();
                if (status == CF_CONFLICT) {
                    return status;
                }
            }
            pathV.pop();
        }
        return CF_STAY;
    }

    //-------------------------------------------------------------------------

    enum DFSColor { WHITE = 0, GRAY = 1, BLACK = 2 };

    int filterMemo(vec<int32_t>& pathV, vec<int32_t>& pathE, vec<int64_t>& pathW,
        int32_t v, int32_t lastEdge, bool definedEdge, vec<DFSColor>& flags)
    {
        if (flags[v] == BLACK) {
            return CF_STAY;
        }

        if (flags[v] == GRAY) {
            int32_t index = findVertex(v, pathV);
            if (!satisfiedConditions(pathV, pathE, pathW, index)) {
                vec<Lit> lits;
                lits.push();
                clausify(pathE, E, lits);
                Clause* reason = Reason_new(lits);
                if (!E[lastEdge].setVal(false, reason)) {
                    return CF_CONFLICT;
                }
            }
            return CF_STAY;
        }

        // flags[v] == WHITE
        if (definedEdge) {
            flags[v] = GRAY;
            pathV.push(v);

            for (size_t i = 0; i < g.outs[v].size(); i++) {
                int32_t e = g.outs[v][i];
                if (E[e].isFalse()) continue;

                int w = g.targets[e];
                int64_t acum = pathW.size() ? g.weights[e] + pathW.last()
                                            : g.weights[e];
                pathE.push(e);
                pathW.push(acum);
                int status = filterMemo(pathV, pathE, pathW, 
                                        w, e, E[e].isTrue(), flags);
                pathW.pop();
                pathE.pop();

                if (status == CF_CONFLICT) {
                    pathV.pop();
                    flags[v] = WHITE;
                    return status;
                }
            }

            pathV.pop();
            flags[v] = BLACK;
        }

        return CF_STAY;
    }

    //-------------------------------------------------------------------------
    
    bool propagate() override {
        vec<int32_t> pathV;
        vec<int32_t> pathE;
        vec<int64_t> pathW;

        // if (filter(pathV,pathE,pathW,g.init,-1,true) == CF_CONFLICT)
        //     return false;
        vec<DFSColor> flags(g.nvertices, WHITE);
        if (filterMemo(pathV,pathE,pathW,g.init,-1,true,flags) == CF_CONFLICT)
            return false;

        return true;
    }

    //-------------------------------------------------------------------------
    
    void wakeup(int i, int) override {
        pushInQueue();
    }
    
    //-------------------------------------------------------------------------
    
    void clearPropState() override {
        in_queue = false;
    }
};

//=============================================================================

class NOCBrancher : public Branching {
private:
    Game&           g;
    vec<BoolView>   V;
    vec<BoolView>   E;
    parity_type     playerSAT;
public:
    
    NOCBrancher(Game& g, vec<BoolView>& V, vec<BoolView>& E, 
        parity_type playerSAT) 
    : g(g), V(V), E(E), playerSAT(playerSAT) {}

    //-------------------------------------------------------------------------
    
    bool finished() override {
        for (size_t i=0; i<V.size(); i++) {
            if (!V[i].isFixed()) return false;
        }
        for (size_t i=0; i<E.size(); i++) {
            if (!E[i].isFixed()) return false;
        }
        return true;
    }

    //-------------------------------------------------------------------------

    double getScore(VarBranch vb) override {
        return 0;
    }

    //-------------------------------------------------------------------------

    DecInfo* branch() override {
        for (size_t v=0; v<V.size(); v++) {
            if (g.owners[v]==playerSAT && V[v].isFixed()) {
                for (size_t j=0; j<g.outs[v].size(); j++) {
                    int32_t e = g.outs[v][j];
                    if (!E[e].isFixed()) {
                        return E[e].branch();
                    }
                }
            }
        }
        return nullptr;
    }
};

//=============================================================================

class NOCModel : public Problem {
private:
    Game&                   g;
    vec<BoolView>           V;
    vec<BoolView>           E;
    vec<WinningCondition*>  winConditions;
    bool                    heuristicReach;
    int                     printtype;
    parity_type             playerSAT;
public:

    //-------------------------------------------------------------------------

    NOCModel(Game& g, vec<WinningCondition*>& winConditions, 
        int printtype=0, parity_type playerSAT=EVEN, bool heuristicReach=false) 
    :g(g), winConditions(winConditions), printtype(printtype), 
        playerSAT(playerSAT), heuristicReach(heuristicReach)
    {
        V.growTo(g.nvertices);
        E.growTo(g.nedges);
        setupConstraints();
    }

    //-------------------------------------------------------------------------

    void setupConstraints() {

        for (size_t i=0; i<g.nvertices;  i++) V[i] = newBoolVar();
        for (size_t i=0; i<g.nedges;     i++) E[i] = newBoolVar();

        // Initial vertex
        fixVertices({g.init},{});

        // --------------------------------------------------------------------
        // For every active PLAYER vertex, one outgoing edge must be activated
        for (int32_t v=0; v<g.nvertices; v++) if (g.owners[v] == playerSAT) {

            int32_t n = g.outs[v].size();

            // --- At least one -----------------------------------------------
            if (n == 0) continue;

            {
                vec<Lit> clause;
                clause.push( V[v].getLit(false) );
                for (size_t i=0; i<g.outs[v].size(); i++) {
                    int32_t e = g.outs[v][i];
                    clause.push(E[e].getLit(true));
                }
                sat.addClause(clause); // E_0 \/ E_1 \/ ... \/ E_n
            }

            // --- At most one ------------------------------------------------
            if (n == 1) continue;

            vec<BoolView> s(n - 1);
            for (size_t j = 0; j < n - 1; j++) s[j] = newBoolVar();

            // First literal
            {
                int32_t e = g.outs[v][0];
                // -E_0 \/ s_0
                vec<Lit> clause;
                clause.push(E[e].getLit(false));
                clause.push(s[0].getLit(true));
                sat.addClause(clause);
            }

            // Middle literals
            for (size_t i = 1; i < n - 1; i++) {
                int32_t e = g.outs[v][i];

                // -s_{i-1} \/ s_i
                {
                    vec<Lit> clause;
                    clause.push(s[i - 1].getLit(false));
                    clause.push(s[i].getLit(true));
                    sat.addClause(clause);
                }

                // -E_i \/ -s_{i-1}
                {
                    vec<Lit> clause;
                    clause.push(E[e].getLit(false));
                    clause.push(s[i - 1].getLit(false));
                    sat.addClause(clause);
                }

                // -E_i \/ s_i
                {
                    vec<Lit> clause;
                    clause.push(E[e].getLit(false));
                    clause.push(s[i].getLit(true));
                    sat.addClause(clause);
                }
            }

            // Last literal
            {
                int32_t e_last = g.outs[v][n - 1];
                // -E_{n-1} \/ -s_{n-2}
                vec<Lit> clause;
                clause.push(E[e_last].getLit(false));
                clause.push(s[n - 2].getLit(false));
                sat.addClause(clause);
            }
        }

        // --------------------------------------------------------------------
        // Every active OPPONENT vertice, each outgoing edge must be activated
        for (size_t v=0; v<g.nvertices; v++) {
            if (g.owners[v]==opponent(playerSAT)) {
                for (size_t i=0; i<g.outs[v].size(); i++) {
                    int32_t e = g.outs[v][i];
                    vec<Lit> clause;
                    clause.push( V[v].getLit(false) );        
                    clause.push( E[e].getLit(true) );
                    sat.addClause(clause);
                }
            }
        }

        // --------------------------------------------------------------------
        // For every active edge, the target vertex must be activated
        for (size_t w=0; w<g.nvertices; w++) if (w != g.init) {
            for (size_t i=0; i<g.ins[w].size(); i++) {
                int32_t e = g.ins[w][i];
                vec<Lit> clause;
                clause.push( E[e].getLit(false) );
                clause.push( V[w].getLit(true) );
                sat.addClause(clause);
            }
        }

        // --------------------------------------------------------------------
        // Every infinite OPPONENT play must be avoided regarding codition.
        // new NOCPropagator(g,V,E,playerSAT,winConditions);
        // new NOCCheckerSCC(g,V,E,playerSAT);
        new NOCCheckerBellmanFord(g,V,E,playerSAT);

        //---------------------------------------------------------------------

        vec<Branching*> bv(static_cast<unsigned int>(g.nvertices));
        vec<Branching*> be(static_cast<unsigned int>(g.nedges));
        for (size_t i = g.nvertices; (i--) != 0;) bv[i] = &V[i];
        for (size_t i = g.nedges;    (i--) != 0;) be[i] = &E[i];
        
        if (heuristicReach) {
            engine.branching->add(new NOCBrancher(g,V,E,playerSAT));
        }
        branch(bv, VAR_INORDER, VAL_MIN);
        branch(be, VAR_INORDER, VAL_MIN);
        output_vars(bv);
        output_vars(be);
    }

    //-------------------------------------------------------------------------

    void fixVertices(   std::initializer_list<int32_t> vs,
                        std::initializer_list<int32_t> nvs={})
    {
        for (int32_t v : vs) {
            vec<Lit> clause;
            clause.push(V[v].getLit(true));
            sat.addClause(clause);
        }
        for (int32_t v : nvs) {
            vec<Lit> clause;
            clause.push(V[v].getLit(false));
            sat.addClause(clause);
        }
    }

    //-------------------------------------------------------------------------

    void fixEdges(  std::initializer_list<int32_t> es,
                    std::initializer_list<int32_t> nes={}) 
    {
        for (int32_t e : es) {
            vec<Lit> clause;
            clause.push(E[e].getLit(true));
            sat.addClause(clause);
        }
        for (int32_t e : nes) {
            vec<Lit> clause;
            clause.push(E[e].getLit(false));
            sat.addClause(clause);
        }
    }

    //-------------------------------------------------------------------------

    void print(std::ostream& out) override {
        if (printtype) {
            out << "V=[";
            bool first = true;
            for (size_t i=0; i<V.size(); i++) {
                if (V[i].isTrue()) {
                    if (first) first=false; else out << ",";
                    out << i;
                }
            }
            out << "]\nE=[";
            first = true;
            for (size_t i=0; i<E.size(); i++) {
                if (E[i].isTrue()) {
                    if (first) first=false; else out << ",";
                    out << i;
                }
            }
            out << "]";
        }
    }
};

} // namespace ChuffedBool