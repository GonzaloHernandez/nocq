#ifndef GAME_H
#include "game.h"
#endif

#include <cmath>
#include <chrono>

//=============================================================================

class Pool {
private:
    int id;
public:
    Pool() : id(0) {}
    int getId() { return ++id; }
    int top()   { return id; }
};

//=============================================================================

class SATEncoder {
private:
    Game& g;
    std::vector<int>    V;
    std::vector<int>    E;
    std::vector<std::vector<std::vector<int>>>    P;
    std::vector<int>    oddcolors;
    Pool                pool;
    //-------------------------------------------------------------------------

    std::vector<std::vector<int>> encode_greaterequal(int a, int b, int p);
    std::vector<std::vector<int>> encode_strictlygreater(int a, int b, int p);
    void calculate_oddcolors();
    int  calculate_nbits(int n);
    void createLiterals();
public:
    SATEncoder(Game& g);
    std::vector<std::vector<int>> getCNF();
    void dimacs(std::vector<std::vector<int>>& cnf, std::string filename);
};

//=============================================================================


