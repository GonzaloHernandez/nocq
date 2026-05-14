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
#include "cadical.hpp"
#include <queue>
#include <unordered_map>

#ifndef WINNING_CONDITIONS_H
#include "winning_conditions.h"
#endif

namespace CaDiCaL {

#define BoolSAT int
#define Lit int

//=============================================================================

class NOCPropagator : public ExternalPropagator {
private:
    Game& g;
    vec<BoolSAT>& V;
    vec<BoolSAT>& E;
    vec<int>    assigns;
    parity_type playerSAT;
    vec<WinningCondition*> winConditions;

    size_t reasonLit;

    std::queue<int> propQueue;
    std::unordered_map<int, vec<int>> propReasons;

    vec<vec<int>> trail;

    int getValLit(int v){ return assigns[v]*v; }
    bool isFixed(int v) { return assigns[v] != 0; }
    bool isFalse(int v) { return assigns[v] == -1; }
    bool isTrue(int v)  { return assigns[v] == 1; }

public:
    NOCPropagator(Game& game, vec<BoolSAT>& V, vec<BoolSAT>& E, 
        parity_type playerSAT, vec<WinningCondition*> winConditions)
    : g(game), V(V), E(E), playerSAT(playerSAT), winConditions(winConditions), 
        reasonLit(0), assigns(g.nvertices+g.nedges+1,0)
    {
        trail.push();
    }
    //-------------------------------------------------------------------------
    ~NOCPropagator() override = default;
    //-------------------------------------------------------------------------
    void notify_assignment(const std::vector<int> &lits) override {
        for (int lit : lits) {
            int v = abs(lit);
            if (assigns[v] != 0) continue;
            assigns[v] = (lit > 0) ? 1 : -1;
            trail.last().push(v);
        }
    }
    //-------------------------------------------------------------------------
    void notify_new_decision_level () override {
        trail.push();
    }
    //-------------------------------------------------------------------------
    void notify_backtrack(size_t new_level) override {
        while (trail.size() > (new_level+1)) {
            for (int i=0; i<trail.last().size(); i++) { 
                int v = trail.last()[i];
                assigns[v] = 0;
            }
            trail.pop();
        }
    }
    //-------------------------------------------------------------------------
    bool cb_check_found_model (const std::vector<int> &model) override {
        return true;
    }
    //-------------------------------------------------------------------------
    int cb_decide() override {
        return 0;
    }
    //-------------------------------------------------------------------------
    int cb_propagate() override {
        if (!propQueue.empty()) {
            int lit = propQueue.front();
            propQueue.pop();
            return lit;
        }

        vec<int32_t> pathV;
        vec<int32_t> pathE;
        vec<int64_t> pathW;

        filter(pathV,pathE,pathW,g.init,-1,true);

        if (!propQueue.empty()) {
            int lit = propQueue.front();
            propQueue.pop();
            return lit;
        }

        return 0;
    }
    //-------------------------------------------------------------------------
    int cb_add_reason_clause_lit(int propagated_lit) override {
        const vec<int>& reason = propReasons[propagated_lit];
        if (reasonLit < reason.size()) {
            int lit = reason[reasonLit++];
            return lit;
        }
        reasonLit = 0;
        return 0;
    }
    //-------------------------------------------------------------------------
    bool cb_has_external_clause(bool &is_forgettable) override {
        return false;
    }
    //-------------------------------------------------------------------------
    int cb_add_external_clause_lit() override {
        return 0;
    }
    //-------------------------------------------------------------------------
    int findVertex(int vertex,vec<int>& path) {
        for (int i=0; i<path.size(); i++) {
            if (path[i] == vertex) return i;
        }
        return -1;
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
    bool filter(vec<int32_t>& pathV, vec<int32_t>& pathE, vec<int64_t>& pathW,
        int32_t v, int32_t lastEdge, bool definedEdge) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {
            if (!satisfiedConditions(pathV,pathE,pathW,index)) {
                propReasons[-E[lastEdge]].clear();
                for (size_t i=0; i<pathV.size(); i++) {
                    propReasons[-E[lastEdge]].push( -E[pathE[i]] );
                }
                reasonLit = 0;
                
                propQueue.push(-E[lastEdge]);
                if (isTrue(E[lastEdge])) {
                    return true;
                }
                return false;
            }
        }
        else if (definedEdge) {
            pathV.push(v);
            for (size_t i=0; i<g.outs[v].size(); i++) {
                int32_t e = g.outs[v][i];
                if (isFalse(E[e])) continue;
                int w = g.targets[e];
                int64_t acum = pathW.size() ? g.weights[e]+pathW.last()
                                            : g.weights[e];
                
                pathE.push(e);
                pathW.push(acum);
                bool confl = filter(pathV, pathE, pathW, w, e, isTrue(E[e]));
                pathW.pop();
                pathE.pop();
                if (confl) {
                    return true;
                }
            }
            pathV.pop();
        }
        return false;
    }
};

//=============================================================================

class NOCModel {
private:
    Game& g;
    vec<BoolSAT> V;
    vec<BoolSAT> E;
    vec<WinningCondition*> winConditions;
    int threshold;
    parity_type playerSAT;

    int pool=0;
    Solver* solver;

    int newBoolVar ()           { pool += 1;    return pool; }
    int newBoolVars(int size)   { pool += size; return pool-size+1; }
public:
    //-------------------------------------------------------------------------
    NOCModel(Game& g, vec<WinningCondition*>& winConditions, 
        parity_type playerSAT=EVEN)
    : g(g), winConditions(winConditions), threshold(threshold), 
        playerSAT(playerSAT) 
    {
        solver = new Solver();
        solver->set("factor",0);

        V.growTo(g.nvertices,0);
        E.growTo(g.nedges,0);

        setupConstraints();

        for (int i=0; i<g.nvertices; i++) {
            solver->add_observed_var(V[i]);
        }
        for (int i=0; i<g.nedges; i++) {
            solver->add_observed_var(E[i]);
        }
    }
    //-------------------------------------------------------------------------
    void setupConstraints() {
        for (int i=0; i<g.nvertices;  i++) V[i] = newBoolVar();
        for (int i=0; i<g.nedges;     i++) E[i] = newBoolVar();

        solver->clause(V[g.init]);
        // --------------------------------------------------------------------
        // For every active PLAYER vertex, one outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == playerSAT) {

            int n = g.outs[v].size();

            // --- At least one -----------------------------------------------
            if (n == 0) continue;

            {                           // V -> E_0 \/ E_1 \/ ... \/ E_n
                solver->add( -V[v] );
                for (size_t i=0; i<g.outs[v].size(); i++) {
                    int32_t e = g.outs[v][i];
                    solver->add( E[e] );
                }
                solver->add(0); 
            }
            
            // --- At most one ------------------------------------------------
            if (n == 1) continue;

            vec<BoolSAT> s(n-1);
            for (int j = 0; j < n-1; j++) s[j] = newBoolVar();

            // First literal
            {
                int e = g.outs[v][0];
                solver->clause( -E[e], s[0] );      // E_0 -> s_0
            }

            // Middle literals
            for (int i = 1; i < n-1; i++) {
                int e = g.outs[v][i];
                solver->clause( -E[e], s[i] );      // E_i -> s_i
                solver->clause( -E[e], -s[i-1] );   // E_i -> -s_{i-1}
                solver->clause( -s[i-1], s[i] );    // s_{i-1} -> s_i
            }

            // Last literal
            {
                int e = g.outs[v][n-1];
                solver->clause( -E[e], -s[n-2] );   // E_{n-1} -> -s_{n-2}
            }
        }
    
        // --------------------------------------------------------------------
        // For every active OPPONENT vertice, each outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v]==opponent(playerSAT)) {
            for (size_t i=0; i<g.outs[v].size(); i++) {
                int32_t e = g.outs[v][i];
                solver->clause( -V[v], E[e] );      // V -> E
            }
        }

        // --------------------------------------------------------------------
        // For every active edge, the target vertex must be activated
        for (int e=0; e<g.nedges; e++) {
            int w = g.targets[e];
            solver->clause( -E[e], V[w] );          // E -> V
        }

        // --------------------------------------------------------------------
        // Every infinite OPPONENT play must be avoided regarding codition.

        NOCPropagator *noc = new NOCPropagator(g,V,E,playerSAT,winConditions);
        solver->connect_external_propagator(noc);
    }
    //-------------------------------------------------------------------------
    ~NOCModel() {
        delete solver;
    }
    //-------------------------------------------------------------------------
    bool solve() {
        int res = solver->solve();

        if (res == 10) {
            return true;
        } else {
            return false;
        }
    }
    //-------------------------------------------------------------------------
    void print() {
        std::cout << "V=[";
        bool first = true;
        for (int i=0; i<V.size(); i++) {
            if (solver->val(V[i]) == 1) {
                if (first) first=false; else std::cout << ",";
                std::cout << i;
            }
        }
        std::cout << "]\nE=[";
        first = true;
        for (int i=0; i<E.size(); i++) {
            if (solver->val(E[i]) == 1) {
                if (first) first=false; else std::cout << ",";
                std::cout << i;
            }
        }
        std::cout << "]"<<std::endl;
    }
    //-------------------------------------------------------------------------
    void statistics() {
        solver->statistics();
    }
};

} // namespace CaDiCaL
