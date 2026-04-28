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
    vec<WinningCondition*> winConditions;
public:
    // ------------------------------------------------------------------------
    NoOpponentCycle(Space& home, Game& g,
                    ViewArray<Int::BoolView> vs,
                    ViewArray<Int::BoolView> es,
                    parity_type playerSAT, vec<WinningCondition*> winConditions)
    :   Propagator(home), g(g), V(vs), E(es), 
        playerSAT(playerSAT), winConditions(winConditions)
    {
        V.subscribe(home, *this, Int::PC_BOOL_VAL);
        E.subscribe(home, *this, Int::PC_BOOL_VAL);
    }
    // ------------------------------------------------------------------------
    static ExecStatus post(Space& home,
        Game& g,
        ViewArray<Int::BoolView> vs,
        ViewArray<Int::BoolView> es,
        parity_type playerSAT, vec<WinningCondition*> winConditions)
    {
        new (home) NoOpponentCycle(home, g, vs, es, playerSAT, winConditions);
        return ES_OK;
    }
    // ------------------------------------------------------------------------
    NoOpponentCycle(Space& home, NoOpponentCycle& source) 
    :   Propagator(home,source), g(source.g),
        playerSAT(source.playerSAT), winConditions(source.winConditions)
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
        vec<int32_t> pathV;
        vec<int32_t> pathE;

        if (filter(home, pathV,pathE,g.init,-1,true) == ES_FAILED)
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
    int findVertex(int32_t vertex,vec<int32_t>& path) {
        for (int32_t i=0; i<path.size(); i++) {
            if (path[i] == vertex) return i;
        }
        return -1;
    }
    //-------------------------------------------------------------------------
    bool satisfiedConditions(   vec<int32_t>& pathV,vec<int32_t>& pathE,
                                vec<int64_t>& pathW,int32_t index) {
        if (playerSAT==EVEN) {
            for (int32_t i=0; i<winConditions.size(); i++) {
                if (!winConditions[i]->satisfy(pathV,pathE,pathW,index)) {
                    return false;
                }
            }
            return true;
        }
        else {
            for (int32_t i=0; i<winConditions.size(); i++) {
                if (winConditions[i]->satisfy(pathV,pathE,pathW,index)) {
                    return true;
                }
            }
            return false;
        }
    }
    //-------------------------------------------------------------------------
    ExecStatus filter(  Space& home, vec<int32_t>& pathV, vec<int32_t>& pathE, 
                        vec<int64_t>& pathW, int32_t v, 
                        int32_t lastEdge, bool definedEdge) 
    {
        int32_t index = findVertex(v,pathV);
        if (index >= 0) {

            if (!satisfiedConditions(pathV,pathE,pathW,index)) {
                if (me_failed(E[lastEdge].zero(home))) {
                    return ES_FAILED;
                }
            }
        }
        else if (definedEdge) {
            pathV.push(v);
            for (size_t i=0; i<g.outs[v].size(); i++) {
                int32_t e = g.outs[v][i];
                if (E[e].zero()) continue;

                int32_t w = g.targets[e];
                int64_t acum = pathW.size() ? g.weights[e]+pathW.last()
                                            : g.weights[e];
                pathE.push(e);
                pathW.push(acum);
                ExecStatus status = 
                    filter(home, pathV, pathE, pathW, w, e, E[e].one());
                pathW.pop();
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
    vec<WinningCondition*>& winConditions;
    int threshold;
    parity_type playerSAT;
public:
    // ------------------------------------------------------------------------
    NocModel(Game& g, vec<WinningCondition*>& winConditions, 
        parity_type playerSAT=EVEN) 
    :   V(*this, g.nvertices, 0, 1),E(*this, g.nedges, 0, 1),
        g(g), winConditions(winConditions), threshold(threshold), playerSAT(playerSAT)
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
        for (size_t v=0; v<g.nvertices; v++) if (g.owners[v] == playerSAT) {
            size_t n = g.outs[v].size();
            BoolVarArgs edgeVars(n);
            for (size_t i = 0; i < n; i++) {
                int32_t e = g.outs[v][i];
                edgeVars[i] = E[e];
            }
            BoolVar sumIsOne(*this, 0, 1);
            linear(*this, edgeVars, IRT_EQ, 1, sumIsOne);
            rel(*this, V[v], BOT_IMP, sumIsOne, 1);
        }

        // --------------------------------------------------------------------
        // For every OPPONENT active vertex, each outgoing edge must be activated
        for (size_t v=0; v<g.nvertices; v++) if (g.owners[v] == opponent(playerSAT)) {
            size_t n = g.outs[v].size();
            for (size_t i = 0; i < n; i++) {
                int32_t e = g.outs[v][i];
                rel(*this, V[v], BOT_IMP, E[e], 1);
            }

        }

        // --------------------------------------------------------------------
        // For every activated edge, the target vertex must be activated
        for (size_t w = 0; w < g.nvertices; w++) {
            if (w != g.init) {
                for (size_t i=0; i<g.ins[w].size(); i++) {
                    int32_t e = g.ins[w][i];
                    rel(*this, E[e], BOT_IMP, V[w], 1);
                }
            }
        }

        // --------------------------------------------------------------------
        // Every infinite OPPONENT play must be avoided regarding codition.
        noopponentcyclegecode(*this,g,V,E,playerSAT,winConditions);
    }

    // ------------------------------------------------------------------------
    NocModel(NocModel& source) 
    : Space(source), g(source.g), winConditions(source.winConditions), 
        threshold(source.threshold), playerSAT(source.playerSAT) 
    {
        V.update(*this, source.V);
        E.update(*this, source.E);
    }

    // ------------------------------------------------------------------------
    virtual Space* copy(void) override {
        return new NocModel(*this);
    }

    // ------------------------------------------------------------------------
    void print() const {
        std::cout << "{";
        bool first = true;
        for (int32_t v=0; v<g.nvertices; v++) {
            if (V[v].val() == 0) continue;
            if (!first) std::cout << ",";
            std::cout << v;
            first = false;
        }
        std::cout << "} {";
        first = true;
        for (int32_t e=0; e<g.nedges; e++) {
            if (E[e].val() == 0) continue;
            if (!first) std::cout << ",";
            std::cout << e;
            first = false;
        }
        std::cout << "}\n";
    }

};

} // namespace Gecode