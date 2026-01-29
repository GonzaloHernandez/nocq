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
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include "iostream"

#ifndef WINNING_CONDITIONS_H
#include "winning_conditions.h"
#endif

namespace Gecode {

//=============================================================================

class NoOpponentCycle : public Propagator {
protected:
    Game& g;
    ViewArray<Int::BoolView> V;
    ViewArray<Int::BoolView> E;
    parity_type playerSAT;
    vec<WinningCondition*> conditions;
public:
    // ------------------------------------------------------------------------
    NoOpponentCycle(Space& home, Game& g,
                    ViewArray<Int::BoolView> vs,
                    ViewArray<Int::BoolView> es,
                    parity_type playerSAT, vec<WinningCondition*> conditions)
    :   Propagator(home), g(g), V(vs), E(es), 
        playerSAT(playerSAT), conditions(conditions)
    {
        V.subscribe(home, *this, Int::PC_BOOL_VAL);
        E.subscribe(home, *this, Int::PC_BOOL_VAL);
    }
    // ------------------------------------------------------------------------
    static ExecStatus post(Space& home,
        Game& g,
        ViewArray<Int::BoolView> vs,
        ViewArray<Int::BoolView> es,
        parity_type playerSAT, vec<WinningCondition*> conditions)
    {
        new (home) NoOpponentCycle(home, g, vs, es, playerSAT, conditions);
        return ES_OK;
    }
    // ------------------------------------------------------------------------
    NoOpponentCycle(Space& home, NoOpponentCycle& source) 
    :   Propagator(home,source), g(source.g),
        playerSAT(source.playerSAT), conditions(source.conditions)
    {
        V.update(home, source.V);
        E.update(home, source.E);
    }
    // ------------------------------------------------------------------------
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
        return PropCost::ternary(PropCost::HI);
    }    
    // ------------------------------------------------------------------------
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
        vec<int> pathV;
        vec<int> pathE;

        if (filterEager(home, pathV,pathE,g.init,-1,true) == ES_FAILED)
            return ES_FAILED;

        return ES_OK;
    }
    // ------------------------------------------------------------------------
    virtual Propagator* copy(Space& home) {
        return new (home) NoOpponentCycle(home, *this);
    }
    //-------------------------------------------------------------------------
    virtual void reschedule(Space& home) {
        V.reschedule(home, *this, Int::PC_BOOL_VAL);
        E.reschedule(home, *this, Int::PC_BOOL_VAL);
    }
    //-------------------------------------------------------------------------
    virtual size_t dispose(Space& home) {
        V.cancel(home, *this, Int::PC_BOOL_VAL);
        E.cancel(home, *this, Int::PC_BOOL_VAL);
        (void) Propagator::dispose(home);
        return sizeof(*this);
    }
    //-------------------------------------------------------------------------
    int findVertex(int vertex,vec<int>& path) {
        for (int i=0; i<path.size(); i++) {
            if (path[i] == vertex) return i;
        }
        return -1;
    }
    //-------------------------------------------------------------------------
    bool satisfiedConditions(vec<int>& pathV,vec<int>& pathE,int index) {
        if (playerSAT==EVEN) {
            for (int i=0; i<conditions.size(); i++) {
                if (!conditions[i]->satisfy(pathV,pathE,index)) {
                    return false;
                }
            }
            return true;
        }
        else {
            for (int i=0; i<conditions.size(); i++) {
                if (conditions[i]->satisfy(pathV,pathE,index)) {
                    return true;
                }
            }
            return false;
        }
    }
    //-------------------------------------------------------------------------
    ExecStatus filterEager( Space& home, 
                            vec<int>& pathV, vec<int>& pathE, int v, 
                            int lastEdge, bool definedEdge) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {

            if (!satisfiedConditions(pathV,pathE,index)) {
                if (me_failed(E[lastEdge].zero(home))) {
                    return ES_FAILED;
                }
            }
        }
        else if (definedEdge) {
            pathV.push(v);
            for (int e : g.outs[v]) {
                if (E[e].zero()) continue;

                int w = g.targets[e];
                pathE.push(e);
                ExecStatus status = 
                    filterEager(home, pathV, pathE, w, e, E[e].one());
                pathE.pop();
                if (status == ES_FAILED) {
                    return status;
                }
            }
            pathV.pop();
        }
        return ES_OK;
    }
};

void noopponentcyclegecode( Space& home, Game& g, 
                            const BoolVarArgs& v, 
                            const BoolVarArgs& e,
                            parity_type playerSAT, 
                            vec<WinningCondition*> conditions)
{
    ViewArray<Int::BoolView> V(home,v);
    ViewArray<Int::BoolView> E(home,e);
    if (NoOpponentCycle::post(home,g,V,E,playerSAT,conditions) != ES_OK)
        home.fail();
}

//=============================================================================

class NocModel : public Space {
protected:
    Game& g;
    BoolVarArray V;
    BoolVarArray E;
    std::vector<bool> conditions;
    int threshold;
    parity_type playerSAT;
public:
    // ------------------------------------------------------------------------
    NocModel(Game& g, std::vector<bool> conditions, int threshold=1, 
        parity_type playerSAT=EVEN) 
    :   V(*this, g.nvertices, 0, 1),E(*this, g.nedges, 0, 1),
        g(g), conditions(conditions), threshold(threshold), playerSAT(playerSAT)
    {
        setupConstraints();
        branch(*this, V, BOOL_VAR_NONE(), BOOL_VAL_MIN());
        branch(*this, E, BOOL_VAR_NONE(), BOOL_VAL_MIN());
    }

    // ------------------------------------------------------------------------
    void setupConstraints() {
        // Initial vertex
        rel(*this, V[g.init], IRT_EQ, 1);

        // --------------------------------------------------------------------
        // For every PLAYER active vertex, one outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == playerSAT) {
            int n = g.outs[v].size();
            BoolVarArgs edgeVars(n);
            for (int i = 0; i < n; i++) {
                int e = g.outs[v][i];
                edgeVars[i] = E[e];
            }
            BoolVar sumIsOne(*this, 0, 1);
            linear(*this, edgeVars, IRT_EQ, 1, sumIsOne);
            rel(*this, V[v], BOT_IMP, sumIsOne, 1);
        }

        // --------------------------------------------------------------------
        // For every OPPONENT active vertex, each outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == opponent(playerSAT)) {
            int n = g.outs[v].size();
            for (int i = 0; i < n; i++) {
                int e = g.outs[v][i];
                rel(*this, V[v], BOT_IMP, E[e], 1);
            }

        }

        // --------------------------------------------------------------------
        // For every activated edge, the target vertex must be activated
        for (int w = 0; w < g.nvertices; w++) {
            if (w != g.init) {
                for (int e : g.ins[w]) {
                    rel(*this, E[e], BOT_IMP, V[w], 1);
                }
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

        noopponentcyclegecode(*this,g,V,E,playerSAT,conds);
    }

    // ------------------------------------------------------------------------
    NocModel(NocModel& source) 
    : Space(source), g(source.g), conditions(source.conditions), 
        threshold(source.threshold), playerSAT(source.playerSAT) 
    {
        V.update(*this, source.V);
        E.update(*this, source.E);
    }

    // ------------------------------------------------------------------------
    virtual Space* copy(void) override {
        return new NocModel(*this);
    }

    void print() const {
        std::cout << "{";
        bool first = true;
        for (int v=0; v<g.nvertices; v++) {
            if (V[v].val() == 0) continue;
            if (!first) std::cout << ",";
            std::cout << v;
            first = false;
        }
        std::cout << "} {";
        first = true;
        for (int e=0; e<g.nedges; e++) {
            if (E[e].val() == 0) continue;
            if (!first) std::cout << ",";
            std::cout << e;
            first = false;
        }
        std::cout << "}\n";
    }

};

} // namespace Gecode