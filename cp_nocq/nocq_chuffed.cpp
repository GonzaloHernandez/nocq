#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"
#include "initializer_list"

#ifndef WINNING_CONDITIONS_H
#include "winning_conditions.h"
#endif

//===========================================================================

class NoOpponentCycle : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    parity_type playerSAT;
    vec<WinningCondition*> conditions;

    const int   CF_DONE     = 1;
    const int   CF_CONFLICT = 2;
    const int   CF_STAY     = 3;

public:
    //-----------------------------------------------------------------------
    NoOpponentCycle(Game& g, vec<BoolView>& V, vec<BoolView>& E, 
        parity_type playerSAT, vec<WinningCondition*> conditions)
    : g(g), V(V), E(E),
        playerSAT(playerSAT), conditions(conditions)
    {
        for (int i=0; i<g.nvertices;i++) V[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<g.nedges;   i++) E[i].attach(this, 1 , EVENT_F );
    }
    //-----------------------------------------------------------------------
    int findVertex(int vertex,vec<int>& path) {
        for (int i=0; i<path.size(); i++) {
            if (path[i] == vertex) return i;
        }
        return -1;
    }
    //-----------------------------------------------------------------------
    void clausify(vec<int>& path, vec<BoolView> &B, vec<Lit>& lits,int from) {
        for (int i=from; i<path.size()-1; i++) {
            lits.push(B[path[i]].getValLit());
        }
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
    int filterEager(vec<int>& pathV, vec<int>& pathE, int v, 
        int lastEdge, bool definedEdge) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {

            if (not satisfiedConditions(pathV,pathE,index)) {
                vec<Lit> lits;
                lits.push();
                clausify(pathE,E,lits,0);
                Clause* reason = Reason_new(lits);
                if (! E[lastEdge].setVal(false,reason)) {
                    return CF_CONFLICT;
                }
            }
        }
        else if (definedEdge) {
            pathV.push(v);
            for (int e : g.outs[v]) {
                if (E[e].isFalse()) continue;

                int w = g.targets[e];
                pathE.push(e);
                int status = filterEager(pathV, pathE, w, e, E[e].isTrue());
                pathE.pop();
                if (status == CF_CONFLICT) {
                    return status;
                }
            }
            pathV.pop();
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    bool propagate() override {
        vec<int> pathV;
        vec<int> pathE;

        if (filterEager(pathV,pathE,g.start,-1,true) == CF_CONFLICT)
            return false;

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


//===========================================================================

class NOCModel : public Problem {
private:
    Game& g;
    vec<BoolView> V;  
    vec<BoolView> E;
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
        E.growTo(g.nedges);
        setupConstraints();
    }

    //----------------------------------------------------------------

    void setupConstraints() {

        for (int i=0; i<g.nvertices;  i++) V[i] = newBoolVar();
        for (int i=0; i<g.nedges;     i++) E[i] = newBoolVar();

        // Starting vertex
        fixVertices({g.start},{});

        // --------------------------------------------------------------
        // For every active PLAYER vertex, exactly one outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == playerSAT) {

            int n = g.outs[v].size();

            // --- At least one ------------------------------------
            if (n == 0) continue;

            {
                vec<Lit> clause;
                clause.push( V[v].getLit(false) );
                for (int e : g.outs[v]) {
                    clause.push(E[e].getLit(true));
                }
                sat.addClause(clause); // E₀ ∨ E₁ ∨ ... ∨ Eₙ
            }

            // --- At most one -------------------------------------
            if (n == 1) continue;

            vec<BoolView> s(n - 1);
            for (int j = 0; j < n - 1; j++) s[j] = newBoolVar();

            // First literal
            {
                int e = g.outs[v][0];
                // -E_0 ∨ s_0
                vec<Lit> clause;
                clause.push(E[e].getLit(false));
                clause.push(s[0].getLit(true));
                sat.addClause(clause);
            }

            // Middle literals
            for (int i = 1; i < n - 1; i++) {
                int e = g.outs[v][i];

                // -s_{i-1} ∨ s_i
                {
                    vec<Lit> clause;
                    clause.push(s[i - 1].getLit(false));
                    clause.push(s[i].getLit(true));
                    sat.addClause(clause);
                }

                // -E_i ∨ ¬s_{i-1}
                {
                    vec<Lit> clause;
                    clause.push(E[e].getLit(false));
                    clause.push(s[i - 1].getLit(false));
                    sat.addClause(clause);
                }

                // -E_i ∨ s_i
                {
                    vec<Lit> clause;
                    clause.push(E[e].getLit(false));
                    clause.push(s[i].getLit(true));
                    sat.addClause(clause);
                }
            }

            // Last literal
            {
                int e_last = g.outs[v][n - 1];
                // -E_{n-1} ∨ -s_{n-2}
                vec<Lit> clause;
                clause.push(E[e_last].getLit(false));
                clause.push(s[n - 2].getLit(false));
                sat.addClause(clause);
            }
        }

        // --------------------------------------------------------------
        // For every active OPPONENT vertice, each outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == opponent(playerSAT)) {
            for (int e : g.outs[v]) {
                vec<Lit> clause;
                clause.push( V[v].getLit(false) );        
                clause.push( E[e].getLit(true) );
                sat.addClause(clause);
            }
        }

        // --------------------------------------------------------------
        // For every active edge, the target vertex must be activated
        for (int w=0; w<g.nvertices; w++) if (w != g.start) {
            for (int e : g.ins[w]) {
                vec<Lit> clause;
                clause.push( E[e].getLit(false) );
                clause.push( V[w].getLit(true) );
                sat.addClause(clause);
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

        new NoOpponentCycle(g,V,E,playerSAT,conds);

        //------------------------------------------------------------

        vec<Branching*> bv(static_cast<unsigned int>(g.nvertices));
        vec<Branching*> be(static_cast<unsigned int>(g.nedges));
        for (int i = g.nvertices; (i--) != 0;) bv[i] = &V[i];
        for (int i = g.nedges;    (i--) != 0;) be[i] = &E[i];
        
        branch(bv, VAR_INORDER, VAL_MIN);
        branch(be, VAR_INORDER, VAL_MIN);
        output_vars(bv);
        output_vars(be);
    }

    //----------------------------------------------------------------

    void fixVertices(std::initializer_list<int> vs,std::initializer_list<int> nvs={}) {
        for (int v : vs) {
            vec<Lit> clause;
            clause.push(V[v].getLit(true));
            sat.addClause(clause);
        }
        for (int v : nvs) {
            vec<Lit> clause;
            clause.push(V[v].getLit(false));
            sat.addClause(clause);
        }
    }

    //----------------------------------------------------------------

    void fixEdges(std::initializer_list<int> es,std::initializer_list<int> nes={}) {
        for (int e : es) {
            vec<Lit> clause;
            clause.push(E[e].getLit(true));
            sat.addClause(clause);
        }
        for (int e : nes) {
            vec<Lit> clause;
            clause.push(E[e].getLit(false));
            sat.addClause(clause);
        }
    }

    //----------------------------------------------------------------

    void print(std::ostream& out) override {
        if (printtype) {
            out << "V=[";
            bool first = true;
            for (int i=0; i<V.size(); i++) {
                if (V[i].isTrue()) {
                    if (first) first=false; else out << ",";
                    out << i;
                }
            }
            out << "]\nE=[";
            first = true;
            for (int i=0; i<E.size(); i++) {
                if (E[i].isTrue()) {
                    if (first) first=false; else out << ",";
                    out << i;
                }
            }
            out << "]";
        }
    }
};

//----------------------------------------------------------------------