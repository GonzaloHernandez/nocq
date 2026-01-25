#include "satencoder.h"

std::vector<std::vector<int>> SATEncoder::encode_greaterequal(int a, 
    int b, int p) 
{
    std::vector<std::vector<int>> cnf;

    int n = calculate_nbits(p)-1;

    auto it = std::find(oddcolors.begin(), oddcolors.end(), p);
    p = std::distance(oddcolors.begin(), it);

    std::vector<int>& A = P[a][p];
    std::vector<int>& B = P[b][p];
    std::vector<int> C;
    std::vector<int> Q;

    for (int i = 0; i < n+1; i++) C.push_back(pool.getId());
    for (int i = 0; i < n;   i++) Q.push_back(pool.getId());

    cnf.push_back({-C[n], +A[n], -B[n]});
    cnf.push_back({+C[n], -A[n]});
    cnf.push_back({+C[n], +B[n]});

    for (int i = n-1; i >= 0; i--) {
        cnf.push_back({-Q[i], -A[i], +B[i]});
        cnf.push_back({+A[i], +Q[i]});
        cnf.push_back({-B[i], +Q[i]});

        cnf.push_back({-C[i], -B[i], +A[i]});
        cnf.push_back({-C[i], -Q[i], +C[i+1]});

        cnf.push_back({+B[i], +Q[i],   C[i]});
        cnf.push_back({+B[i], -C[i+1], C[i]});
        cnf.push_back({-A[i], +Q[i],   C[i]});
        cnf.push_back({-A[i], -C[i+1], C[i]});
    }

    cnf.push_back({C[0]});

    return cnf;
} 

//-----------------------------------------------------------------------------

std::vector<std::vector<int>> SATEncoder::encode_strictlygreater(int a, 
    int b, int p) 
{
    std::vector<std::vector<int>> cnf;

    int n = calculate_nbits(p)-1;

    auto it = std::find(oddcolors.begin(), oddcolors.end(), p);
    p = std::distance(oddcolors.begin(), it);

    std::vector<int>& A = P[a][p];
    std::vector<int>& B = P[b][p];
    std::vector<int> C;
    std::vector<int> Q;

    for (int i = 0; i < n+1; i++) C.push_back(pool.getId());
    for (int i = 0; i < n;   i++) Q.push_back(pool.getId());
            
    cnf.push_back({-C[n], +A[n]});
    cnf.push_back({-C[n], -B[n]});
    cnf.push_back({+C[n], -A[n], +B[n]});
    
    for (int i = n-1; i >= 0; i--) {
        cnf.push_back({-Q[i], -A[i], +B[i]});
        cnf.push_back({+A[i], +Q[i]});
        cnf.push_back({-B[i], +Q[i]});

        cnf.push_back({-C[i], -B[i], +A[i]});
        cnf.push_back({-C[i], -Q[i], +C[i+1]});

        cnf.push_back({+B[i], +Q[i],   C[i]});
        cnf.push_back({+B[i], -C[i+1], C[i]});
        cnf.push_back({-A[i], +Q[i],   C[i]});
        cnf.push_back({-A[i], -C[i+1], C[i]});
    }

    cnf.push_back({C[0]});

    return cnf;
}    

//-----------------------------------------------------------------------------

void SATEncoder::calculate_oddcolors() {
    int sum = 0;
    for (int v = 0; v < g.nvertices; v++) if (g.priors[v]%2 == ODD) {
        if (!std::binary_search(oddcolors.begin(), oddcolors.end(), g.priors[v])) {
            auto pos = 
                std::lower_bound(oddcolors.begin(), oddcolors.end(), g.priors[v]);
            oddcolors.insert(pos, g.priors[v]);
        }
    }
}

//-----------------------------------------------------------------------------

int SATEncoder::calculate_nbits(int n) {
    int sum = 0;
    for (int v = 0; v < g.nvertices; v++) if (g.priors[v] == n) sum++;
    return std::ceil(std::log2(sum + 1));
}

//-----------------------------------------------------------------------------

void SATEncoder::createLiterals() {
    calculate_oddcolors();

    for (int v = 0; v < g.nvertices; v++) {
        V.push_back(pool.getId());
        std::vector<std::vector<int>> ps;
        for(int p = 0; p < oddcolors.size(); p++) {
            std::vector<int> ms;
            for (int m = 0; m < calculate_nbits(oddcolors[p]); m++) {
                ms.push_back(pool.getId());
            }
            ps.push_back(ms);
        }
        P.push_back(ps);
    }
    for (int e = 0; e < g.nedges; e++) E.push_back(pool.getId());
}

//-----------------------------------------------------------------------------

SATEncoder::SATEncoder(Game& g) : g(g) {
    createLiterals();
}
//-----------------------------------------------------------------------------

std::vector<std::vector<int>> SATEncoder::getCNF() {
    std::vector<std::vector<int>> cnf;

    // Starting vertex
    cnf.push_back({V[g.init]});

    // For every EVEN vertice, at least one outgoing edge must be activated
    for (int v=0; v<g.nvertices; v++) if (g.owners[v] == EVEN) {
        std::vector<int> clause;
        clause.push_back( -V[v] );
        for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
            clause.push_back( E[e] );
        }
        cnf.push_back(clause);
    }

    // For every ODD vertice, each outgoing edge must be activated
    for (int v=0; v<g.nvertices; v++) if (g.owners[v] == ODD) {
        for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
        std::vector<int> clause;
            clause.push_back( -V[v] );        
            clause.push_back( E[e] );
            cnf.push_back(clause);
        }
    }

    // For every activated edge, the source vertex must be activated
    for (int v=0; v<g.nvertices; v++) {
        for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
            std::vector<int> clause;
            clause.push_back( -E[e] );
            clause.push_back( V[v] );
            cnf.push_back(clause);
        }
    }

    // For every activated edge, the target vertex must be activated
    for (int w=0; w<g.nvertices; w++) if (w != g.init) {
        for (int e=0; e<g.nedges; e++) if (g.targets[e]==w) {
            std::vector<int> clause;
            clause.push_back( -E[e] );
            clause.push_back( V[w] );
            cnf.push_back(clause);
        }
    }    

    // Progress measure constraints
    for (int e=0; e<g.nedges; e++) {
        int q = g.priors[g.targets[e]];

        std::vector<std::vector<int>> pm_cnf;

        for (auto& p : oddcolors) if (p < q) {
            std::vector<std::vector<int>> ge = 
                encode_greaterequal(g.targets[e], g.sources[e], p);
            pm_cnf.insert(pm_cnf.end(), ge.begin(), ge.end());
        }

        if (q % 2 == EVEN) {
            for (auto& clause : pm_cnf) {
                std::vector<int> new_clause = clause;
                new_clause.push_back(-E[e]);
                cnf.push_back(new_clause);
            }
        }
        else {
            for (auto& clause : pm_cnf) {
                std::vector<int> new_clause = clause;
                new_clause.push_back(-E[e]);
                cnf.push_back(new_clause);
            }

            std::vector<std::vector<int>> ge = 
                encode_strictlygreater(g.targets[e], g.sources[e], q);
            for (auto& clause : ge) {
                std::vector<int> new_clause = clause;
                new_clause.push_back(-E[e]);
                cnf.push_back(new_clause);
            }
        }
    }

    return cnf;
}

//-----------------------------------------------------------------------------

void SATEncoder::dimacs(std::vector<std::vector<int>>& cnf, 
                        std::string filename) 
{
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file!" << std::endl;
        return;
    }

    file << "p cnf " << pool.top() << " " << cnf.size() << std::endl;
    for (auto& clause : cnf) {
        for (auto& literal : clause) {
            file << literal << " ";
        }
        file << "0" << std::endl;
    }
    file.close();
}

