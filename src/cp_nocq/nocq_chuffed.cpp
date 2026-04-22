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

namespace Chuffed {

//=============================================================================

class NoOpponentCycle : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    parity_type playerSAT;
    vec<WinningCondition*> conditions;

    const int   CF_STAY     = 1;
    const int   CF_CONFLICT = 2;
    const int   CF_DONE     = 3;

public:
    //-------------------------------------------------------------------------
    NoOpponentCycle(Game& g, vec<BoolView>& V, vec<BoolView>& E, 
        parity_type playerSAT, vec<WinningCondition*> conditions)
    : g(g), V(V), E(E), playerSAT(playerSAT), conditions(conditions)
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
    bool satisfiedConditions(vec<int32_t>& pathV,vec<int32_t>& pathE,
        int32_t index) 
    {
        if (playerSAT==EVEN) {
            for (size_t i=0; i<conditions.size(); i++) {
                if (!conditions[i]->satisfy(pathV,pathE,index)) {
                    return false;
                }
            }
            return true;
        }
        else {
            for (size_t i=0; i<conditions.size(); i++) {
                if (conditions[i]->satisfy(pathV,pathE,index)) {
                    return true;
                }
            }
            return false;
        }
    }
    //-------------------------------------------------------------------------
    int filter(vec<int32_t>& pathV, vec<int32_t>& pathE, int32_t v, 
        int32_t lastEdge, bool definedEdge) 
    {
        int32_t index = findVertex(v,pathV);
        if (index >= 0) {

            if (!satisfiedConditions(pathV,pathE,index)) {
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
                pathE.push(e);
                int status = filter(pathV, pathE, w, e, E[e].isTrue());
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
    bool propagate() override {
        vec<int32_t> pathV;
        vec<int32_t> pathE;

        if (filter(pathV,pathE,g.init,-1,true) == CF_CONFLICT)
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

class NOCModel : public Problem {
private:
    Game& g;
    vec<BoolView> V;  
    vec<BoolView> E;
    vec<bool>& conditions;
    float threshold;
    int printtype;
    parity_type playerSAT;
public:

    NOCModel(Game& g, vec<bool>& conditions, float threshold=1, 
        int printtype=0, parity_type playerSAT=EVEN) 
    :g(g), conditions(conditions), threshold(threshold), printtype(printtype), 
        playerSAT(playerSAT)
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
        // For every active OPPONENT vertice, each outgoing edge must be activated
        for (size_t v=0; v<g.nvertices; v++) if (g.owners[v]==opponent(playerSAT)) {
            for (size_t i=0; i<g.outs[v].size(); i++) {
                int32_t e = g.outs[v][i];
                vec<Lit> clause;
                clause.push( V[v].getLit(false) );        
                clause.push( E[e].getLit(true) );
                sat.addClause(clause);
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
        WinningCondition* cond = nullptr;

        vec<WinningCondition*> conds;

        if (conditions[0]) {
            ParityCondition* c = new ParityCondition(g,playerSAT);
            conds.push(c);
        }
            
        if (conditions[1]) {
            EnergyCondition* c = new EnergyCondition(g,playerSAT);
            conds.push(c);
        }
            
        if (conditions[2]) {
            MeanPayoffCondition* c = new MeanPayoffCondition(g,playerSAT);
            c->setThreshold(threshold);
            conds.push(c);
        }

        new NoOpponentCycle(g,V,E,playerSAT,conds);

        //---------------------------------------------------------------------

        vec<Branching*> bv(static_cast<unsigned int>(g.nvertices));
        vec<Branching*> be(static_cast<unsigned int>(g.nedges));
        for (size_t i = g.nvertices; (i--) != 0;) bv[i] = &V[i];
        for (size_t i = g.nedges;    (i--) != 0;) be[i] = &E[i];
        
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

} // namespace Chuffed