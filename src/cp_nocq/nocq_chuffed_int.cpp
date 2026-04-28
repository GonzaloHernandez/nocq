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

namespace ChuffedInt {

//=============================================================================

class NoOpponentCycle : public Propagator {
private:
    Game& g;
    vec<IntVar*> V;
    parity_type playerSAT;
    vec<WinningCondition*> winConditions;

    const int CF_STAY     = 1;
    const int CF_CONFLICT = 2;
    const int CF_DONE     = 3;

public:
    //-------------------------------------------------------------------------
    NoOpponentCycle(Game& g, vec<IntVar*>& V, 
        parity_type playerSAT, vec<WinningCondition*> winConditions)
    : g(g), V(V), playerSAT(playerSAT), winConditions(winConditions)
    {
        for (size_t i=0; i<g.nvertices;i++) V[i]->attach(this, 1 , EVENT_F );
    }
    //-------------------------------------------------------------------------
    int32_t findVertex(int vertex,vec<int>& path) {
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
        int32_t v, int32_t last, int32_t out, bool isFixed) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {
            if (!satisfiedConditions(pathV,pathE,pathW,index)) {
                Clause* r = Reason_new(pathV.size());
                for (size_t i=0; i<pathV.size()-1; i++) {
                    int32_t v_ = pathV[i];
                    (*r)[i+1] = V[v_]->getValLit();
                }
                if (g.owners[last]==playerSAT) {
                    if (!V[last]->remVal(out,r)) return CF_CONFLICT;
                    return CF_STAY;
                } else {
                    if (!V[last]->setVal( -1,r)) return CF_CONFLICT;
                    return CF_DONE;
                }
            }
        }
        else if (isFixed) {
            pathV.push(v);
            if (g.owners[v]==playerSAT) {
                int i = V[v]->getVal();
                int32_t e = g.outs[v][i];
                int32_t w = g.targets[e];
                if (!V[w]->isFixed() || V[w]->getVal()>=0) {
                    int64_t acum = pathW.size() ? g.weights[e]+pathW.last()
                                                : g.weights[e];
                    pathE.push(e);
                    pathW.push(acum);
                    int status = filter(pathV, pathE, pathW,  w, v, i, 
                                        V[w]->isFixed());
                    pathW.pop();
                    pathE.pop();
                    if (status != CF_STAY) return status;
                }
            } else {
                for (int i=0; i<g.outs[v].size(); i++) {
                    int32_t e = g.outs[v][i];
                    int32_t w = g.targets[e];
                    if (!V[w]->isFixed() || V[w]->getVal()>=0) {
                        int64_t acum = pathW.size() ? g.weights[e]+pathW.last()
                                                    : g.weights[e];
                        pathE.push(e);
                        pathW.push(acum);
                        int status = filter(pathV, pathE, pathW, w, v, i, 
                                            V[w]->isFixed());
                        pathW.pop();
                        pathE.pop();
                        if (status != CF_STAY) return status;
                    }
                }
            }
            pathV.pop();
        }
        return CF_STAY;
    }
    //-------------------------------------------------------------------------
    bool propagate() override {
        vec<int32_t> pathV;
        vec<int32_t> pathO;
        vec<int64_t> pathW;

        if (!V[g.init]->isFixed()) return true;

        if (filter(pathV,pathO,pathW,g.init,-1,-1,true) == CF_CONFLICT)
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
    vec<IntVar*> V;
    vec<WinningCondition*>& winConditions;
    int printtype;
    parity_type playerSAT;
public:

    NOCModel(Game& g, vec<WinningCondition*>& winConditions, 
        int printtype=0, parity_type playerSAT=EVEN) 
    :g(g), winConditions(winConditions), printtype(printtype), 
        playerSAT(playerSAT)
    {
        V.growTo(g.nvertices);
        setupConstraints();
    }

    //-------------------------------------------------------------------------

    void setupConstraints() {

        // Creating variables and fixing domains for playerSAT's vertices
        for (size_t v=0; v<g.nvertices; v++) {
            if (g.owners[v]==playerSAT) {
                createVar(V[v],-1,g.outs[v].size()-1);
            } else { // opponent' vertices
                createVar(V[v],-1,0);
            }
        }

        // Initial vertex (V_init = true))
        int_rel(V[g.init], IRT_GE, 0);

        // Connection vertices (V_v -> V_w)
        for (size_t v=0; v<g.nvertices; v++) {
            if (g.owners[v] == playerSAT) {
                for (size_t i=0; i<g.outs[v].size(); i++) {
                    int32_t e = g.outs[v][i];
                    int32_t w = g.targets[e];
                    BoolView sel = newBoolVar();
                    int_rel_reif(V[v], IRT_EQ, i, sel);
                    BoolView geq = newBoolVar();
                    int_rel_reif(V[w], IRT_GE, 0, geq);
                    vec<Lit> clause;
                    clause.push(sel.getLit(false));
                    clause.push(geq.getLit(true));
                    sat.addClause(clause);
                }
            }
            else {
                BoolView sel = newBoolVar();
                int_rel_reif(V[v],IRT_GE, 0, sel);
                for (size_t i=0; i<g.outs[v].size(); i++) {
                    int32_t e = g.outs[v][i];
                    int32_t w = g.targets[e];
                    BoolView geq = newBoolVar();
                    int_rel_reif(V[w], IRT_GE,  0, geq);
                    vec<Lit> clause;
                    clause.push(sel.getLit(false));
                    clause.push(geq.getLit(true));
                    sat.addClause(clause);
                }
            }
        }

        // --------------------------------------------------------------------
        // Every infinite OPPONENT play must be avoided regarding codition.
        new NoOpponentCycle(g,V,playerSAT,winConditions);

        //---------------------------------------------------------------------

        branch(V, VAR_INORDER, VAL_MIN);
        output_vars(V);
    }

    //-------------------------------------------------------------------------

    void print(std::ostream& out) override {
        if (printtype) {
            std::vector<int32_t> vertices;
            std::vector<int32_t> edges;
            for (size_t v=0; v<V.size(); v++) {
                if (V[v]->getVal() < 0) continue;

                vertices.push_back(v);
                if (g.owners[v]==playerSAT) {
                    for (size_t i=0; i<g.outs[v].size(); i++) {
                        int32_t e = g.outs[v][i];
                        if (g.targets[e]==V[v]->getVal()) {
                            edges.push_back(e);
                            break;
                        }
                    }
                } else {
                    for (size_t i=0; i<g.outs[v].size(); i++) {
                        int32_t e = g.outs[v][i];
                        edges.push_back(e);
                    }
                }
            }

            out << "V=[";
            for (size_t v=0 ; v < vertices.size(); v++) {
                out << (v?",":"") << vertices[v];
            }
            out << "]\nE=[";
            for (size_t e=0 ; e < edges.size(); e++) {
                out << (e?",":"") << edges[e];
            }
            out << "]";
        }

    }
};

} // namespace ChuffedInt