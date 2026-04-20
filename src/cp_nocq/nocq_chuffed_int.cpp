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
    vec<WinningCondition*> conditions;

    const int CF_STAY     = 1;
    const int CF_CONFLICT = 2;
    const int CF_DONE     = 3;

public:
    //-------------------------------------------------------------------------
    NoOpponentCycle(Game& g, vec<IntVar*>& V, 
        parity_type playerSAT, vec<WinningCondition*> conditions)
    : g(g), V(V), playerSAT(playerSAT), conditions(conditions)
    {
        for (int i=0; i<g.nvertices;i++) V[i]->attach(this, 1 , EVENT_F );
    }
    //-------------------------------------------------------------------------
    int findVertex(int vertex,vec<int>& path) {
        for (int i=0; i<path.size(); i++) {
            if (path[i] == vertex) return i;
        }
        return -1;
    }
    //-------------------------------------------------------------------------
    void clausify(vec<int>& path, vec<BoolView> &B, vec<Lit>& lits,int from) {
        for (int i=from; i<path.size()-1; i++) {
            lits.push(B[path[i]].getValLit());
        }
    }
    //-------------------------------------------------------------------------
    bool satisfiedConditions(vec<int>& pathV, vec<int>& pathE, int index) {
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
    int filter(vec<int>& pathV, vec<int>& pathE, int v, int last) {
        int index = findVertex(v,pathV);
        if (index >= 0) {
            if (!satisfiedConditions(pathV,pathE,index)) {
                Clause* r = Reason_new(pathV.size());
                for (int i=0; i<pathV.size()-1; i++) {
                    int w = pathV[i];
                    (*r)[i+1] = V[w]->getValLit();
                }
                if (g.owners[last]==playerSAT) {
                    if (!V[last]->remVal( v,r)) return CF_CONFLICT;
                    return CF_STAY;
                } else {
                    if (!V[last]->setVal(-1,r)) return CF_CONFLICT;
                    return CF_DONE;
                }
            }
        }
        else {
            if (!V[v]->isFixed()) return CF_STAY;
            pathV.push(v);
            int outs = g.outs[v].size();
            for (int i=0; i<outs; i++) {
                int e = g.outs[v][i];
                int w = g.targets[e];
                if (g.owners[v]==playerSAT and V[v]->getVal()!=w) continue;
                pathE.push(e);
                int status = filter(pathV, pathE, w, v);
                pathE.pop();
                if (status != CF_STAY) return status;
            }
            pathV.pop();
        }
        return CF_STAY;
    }
    //-------------------------------------------------------------------------
    bool propagate() override {
        vec<int> pathV;
        vec<int> pathE;

        if (filter(pathV,pathE,g.init,-1) == CF_CONFLICT)
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
    std::vector<bool> conditions;
    int threshold;
    int printtype;
    parity_type playerSAT;
public:

    NOCModel(Game& g, std::vector<bool> conditions, int threshold=1, 
        int printtype=0, parity_type playerSAT=EVEN) 
    :g(g), conditions(conditions), threshold(threshold), printtype(printtype), 
        playerSAT(playerSAT)
    {
        V.growTo(g.nvertices);
        setupConstraints();
    }

    //-------------------------------------------------------------------------

    void setupConstraints() {

        // Creating variables and fixing domains for playerSAT's vertices
        for (int v=0; v<g.nvertices; v++) {
            if (g.owners[v]==playerSAT) {
                // (V_v in {v.outs})
                vec<int> values;
                values.push(-1);
                for (int i=0; i<g.outs[v].size(); i++) {
                    int e = g.outs[v][i];
                    int w = g.targets[e];
                    values.push(w);
                }
                V[v] = newIntVar(-1,values.last());
                new (V[v]) IntVarSL(*V[v], values);
            } else { // opponent' vertices
                createVar(V[v],-1,0);
            }
        }

        // Initial vertex (V_init = true))
        int_rel(V[g.init], IRT_GE, 0);

        // Connection vertices (V_v -> V_w)
        for (int v=0; v<g.nvertices; v++) {
            if (g.owners[v] == playerSAT) {
                for (int w=0; w<g.nvertices; w++) {
                    BoolView sel = newBoolVar();
                    int_rel_reif(V[v], IRT_EQ, w, sel);
                    BoolView geq = newBoolVar();
                    int_rel_reif(V[w], IRT_GE,  0, geq);
                    vec<Lit> clause;
                    clause.push(sel.getLit(false));
                    clause.push(geq.getLit(true));
                    sat.addClause(clause);
                }
            }
            else {
                BoolView sel = newBoolVar();
                int_rel_reif(V[v],IRT_GE, 0, sel);
                for (int i=0; i<g.outs[v].size(); i++) {
                    int e = g.outs[v][i];
                    int w = g.targets[e];
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

        new NoOpponentCycle(g,V,playerSAT,conds);

        //---------------------------------------------------------------------

        branch(V, VAR_INORDER, VAL_MIN);
        output_vars(V);
    }

    //-------------------------------------------------------------------------

    void print(std::ostream& out) override {
        if (printtype) {
            std::vector<int> vertices;
            std::vector<int> edges;
            for (int v=0; v<V.size(); v++) {
                if (V[v]->getVal() < 0) continue;

                vertices.push_back(v);
                if (g.owners[v]==playerSAT) {
                    for (int e: g.outs[v]) {
                        if (g.targets[e]==V[v]->getVal()) {
                            edges.push_back(e);
                            break;
                        }
                    }
                } else {
                    for (int e: g.outs[v]) {
                        edges.push_back(e);
                    }
                }
            }

            out << "V=[";
            for (int v=0 ; v < vertices.size(); v++) {
                out << (v?",":"") << vertices[v];
            }
            out << "]\nE=[";
            for (int e=0 ; e < edges.size(); e++) {
                out << (e?",":"") << edges[e];
            }
            out << "]";
        }

    }
};

} // namespace ChuffedInt