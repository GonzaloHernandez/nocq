#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include "iostream"

#ifndef WINNING_CONDITIONS_H
#include "winning_conditions.h"
#endif


//===========================================================================

class NoOpponentCycleGecode : public Gecode::Propagator {
protected:
    Game& g;
    Gecode::ViewArray<Gecode::Int::BoolView> V;
    Gecode::ViewArray<Gecode::Int::BoolView> E;
    parity_type playerSAT;
    vec<WinningCondition*> conditions;
public:
    // --------------------------------------------------------------
    NoOpponentCycleGecode(Gecode::Space& home, Game& g,
        Gecode::ViewArray<Gecode::Int::BoolView> vs,
        Gecode::ViewArray<Gecode::Int::BoolView> es,
        parity_type playerSAT, vec<WinningCondition*> conditions)
    : Gecode::Propagator(home), g(g), V(vs), E(es),
        playerSAT(playerSAT), conditions(conditions)
    {
        V.subscribe(home, *this, Gecode::Int::PC_BOOL_VAL);
        E.subscribe(home, *this, Gecode::Int::PC_BOOL_VAL);
    }
    // --------------------------------------------------------------
    static Gecode::ExecStatus post(Gecode::Space& home,
        Game& g,
        Gecode::ViewArray<Gecode::Int::BoolView> vs,
        Gecode::ViewArray<Gecode::Int::BoolView> es,
        parity_type playerSAT, vec<WinningCondition*> conditions)
    {
        (void) new (home) NoOpponentCycleGecode(home, g, vs, es, playerSAT, conditions);
        return Gecode::ES_OK;
    }
    // --------------------------------------------------------------
    NoOpponentCycleGecode(Gecode::Space& home, NoOpponentCycleGecode& source) 
    : Gecode::Propagator(home,source), g(source.g),
        playerSAT(source.playerSAT), conditions(source.conditions)
    {
        V.update(home, source.V);
        E.update(home, source.E);
    }
    // --------------------------------------------------------------
    virtual Gecode::PropCost cost(const Gecode::Space&, const Gecode::ModEventDelta&) const {
        return Gecode::PropCost::ternary(Gecode::PropCost::HI);
    }    
    // --------------------------------------------------------------
    virtual Gecode::ExecStatus propagate(Gecode::Space& home, const Gecode::ModEventDelta&) {
        vec<int> pathV;
        vec<int> pathE;

        if (filterEager(home, pathV,pathE,g.start,-1,true) == Gecode::ES_FAILED)
            return Gecode::ES_FAILED;

        return Gecode::ES_OK;
    }
    // --------------------------------------------------------------
    virtual Gecode::Propagator* copy(Gecode::Space& home) {
        return new (home) NoOpponentCycleGecode(home, *this);
    }
    //-----------------------------------------------------
    virtual void reschedule(Gecode::Space& home) {
        V.reschedule(home, *this, Gecode::Int::PC_BOOL_VAL);
        E.reschedule(home, *this, Gecode::Int::PC_BOOL_VAL);
    }
    //-----------------------------------------------------------------------
    virtual size_t dispose(Gecode::Space& home) {
        V.cancel(home, *this, Gecode::Int::PC_BOOL_VAL);
        E.cancel(home, *this, Gecode::Int::PC_BOOL_VAL);
        (void) Propagator::dispose(home);
        return sizeof(*this);
    }
    //-----------------------------------------------------------------------
    int findVertex(int vertex,vec<int>& path) {
        for (int i=0; i<path.size(); i++) {
            if (path[i] == vertex) return i;
        }
        return -1;
    }
    //-----------------------------------------------------------------------
    bool satisfiedConditions(vec<int>& pathV,vec<int>& pathE,int index) {
        if (playerSAT==EVEN) {
            for (int i=0; i<conditions.size(); i++) {
                if (not conditions[i]->satisfy(pathV,pathE,index)) {
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
    //-----------------------------------------------------
    Gecode::ExecStatus filterEager(Gecode::Space& home, vec<int>& pathV, vec<int>& pathE, int v, 
        int lastEdge, bool definedEdge) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {

            if (not satisfiedConditions(pathV,pathE,index)) {
                if (Gecode::me_failed(E[lastEdge].zero(home))) {
                    return Gecode::ES_FAILED;
                }
            }
        }
        else if (definedEdge) {
            pathV.push(v);
            for (int e : g.outs[v]) {
                if (E[e].zero()) continue;

                int w = g.targets[e];
                pathE.push(e);
                Gecode::ExecStatus status = filterEager(home, pathV, pathE, w, e, E[e].one());
                pathE.pop();
                if (status == Gecode::ES_FAILED) {
                    return status;
                }
            }
            pathV.pop();
        }
        return Gecode::ES_OK;
    }
};

void noopponentcyclegecode(Gecode::Space& home, Game& g, const Gecode::BoolVarArgs& v, const Gecode::BoolVarArgs& e,
    parity_type playerSAT, vec<WinningCondition*> conditions)
{
    Gecode::ViewArray<Gecode::Int::BoolView> V(home,v);
    Gecode::ViewArray<Gecode::Int::BoolView> E(home,e);
    if (NoOpponentCycleGecode::post(home,g,V,E,playerSAT,conditions) != Gecode::ES_OK) home.fail();
}

//===========================================================================

class NocModelGecode : public Gecode::Space {
protected:
    Game& g;
    Gecode::BoolVarArray V;
    Gecode::BoolVarArray E;
    std::vector<bool> conditions;
    int threshold;
    parity_type playerSAT;
public:
    // --------------------------------------------------------------
    NocModelGecode(Game& g, std::vector<bool> conditions, int threshold=1, 
        parity_type playerSAT=EVEN) 
    : V(*this, g.nvertices, 0, 1),E(*this, g.nedges, 0, 1),
        g(g), conditions(conditions), threshold(threshold), playerSAT(playerSAT)
    {
        setupConstraints();
        Gecode::branch(*this, V, Gecode::BOOL_VAR_NONE(), Gecode::BOOL_VAL_MIN());
        Gecode::branch(*this, E, Gecode::BOOL_VAR_NONE(), Gecode::BOOL_VAL_MIN());
    }

    // --------------------------------------------------------------
    void setupConstraints() {
        // Starting vertex
        Gecode::rel(*this, V[g.start], Gecode::IRT_EQ, 1);

        // --------------------------------------------------------------
        // For every PLAYER active vertex, exactly one outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == playerSAT) {
            int n = g.outs[v].size();
            Gecode::BoolVarArgs edgeVars(n);
            for (int i = 0; i < n; i++) {
                int e = g.outs[v][i];
                edgeVars[i] = E[e];
            }
            Gecode::BoolVar sumIsOne(*this, 0, 1);
            Gecode::linear(*this, edgeVars, Gecode::IRT_EQ, 1, sumIsOne);
            Gecode::rel(*this, V[v], Gecode::BOT_IMP, sumIsOne, 1);
        }

        // --------------------------------------------------------------
        // For every OPPONENT active vertex, each outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == opponent(playerSAT)) {
            int n = g.outs[v].size();
            for (int i = 0; i < n; i++) {
                int e = g.outs[v][i];
                Gecode::rel(*this, V[v], Gecode::BOT_IMP, E[e], 1);
            }

        }

        // --------------------------------------------------------------
        // For every activated edge, the target vertex must be activated
        for (int w = 0; w < g.nvertices; w++) {
            if (w != g.start) {
                for (int e : g.ins[w]) {
                    Gecode::rel(*this, E[e], Gecode::BOT_IMP, V[w], 1);
                }
            }
        }

        // --------------------------------------------------------------
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

    // --------------------------------------------------------------
    NocModelGecode(NocModelGecode& source) 
    : Gecode::Space(source), g(source.g), conditions(source.conditions), 
        threshold(source.threshold), playerSAT(source.playerSAT) 
    {
        V.update(*this, source.V);
        E.update(*this, source.E);
    }

    // --------------------------------------------------------------
    virtual Gecode::Space* copy(void) override {
        return new NocModelGecode(*this);
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
