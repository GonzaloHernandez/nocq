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
#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <vector>
#include <fstream>
#include <regex>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <random>
#include <chrono> 
#include <climits>

#ifndef vec_h
#include "chuffed/support/vec.h"
#endif

enum parity_type    {EVEN,ODD};                             // 0,1
enum objective_type {MIN,MAX};                              // 0,1
enum game_type      {DEF,JURD,RAND,MLADDER,DZN,GM,GMW,DIM}; // 0,1,2,3,4,5,6
enum parity_comp    {BET,EQU,BEQ};

//-----------------------------------------------------------------------------

parity_type opponent(parity_type PARITY);

//=============================================================================

class Game {
public:
    friend class SATEncoder;
    friend class CPModel;
    friend int main(int, char*[]);
public:
    vec<int8_t>         owners;
    vec<int64_t>        priors;
    vec<int32_t>        sources;
    vec<int32_t>        targets;
    vec<float>          weights;

    vec<vec<int32_t>>   outs;
    vec<vec<int32_t>>   ins;
    int32_t             nvertices;
    int32_t             nedges;
    int32_t             init;
    objective_type      objective;

    //-------------------------------------------------------------------------

    void fixZeros();
    void parseline_dzn (const std::string& line,vec<int8_t >& myvec);
    void parseline_dzn (const std::string& line,vec<int32_t>& myvec);
    void parseline_dzn (const std::string& line,vec<int64_t>& myvec);
    void parseline_dzn (const std::string& line,vec<float  >& myvec);

    bool parseline_gm  (const std::string& line,
                        int32_t         vId,
                        int64_t         vPriority,
                        int8_t          vOwner,
                        vec<int32_t>&   vOuts,
                        std::string&    vComment,
                        vec<float>&     outsWeights);

    //-------------------------------------------------------------------------

public:
    
    Game(   vec<int8_t>&    owners,
            vec<int64_t>&   priors,
            vec<int32_t>&   sources,
            vec<int32_t>&   targets,
            vec<float>&     weights,
            int32_t         init = 0, 
            objective_type  obj = MAX);

    Game(   game_type       type, 
            std::string     filename,
            int32_t         init = 0, 
            objective_type  obj = MAX,
            float           lbound = 0.0,
            float           ubound = 1.0);

    Game(   game_type       type,
            vec<int32_t>&   vals,
            vec<float>&     weights,
            int32_t         init = 0,
            objective_type  obj = MAX);

    void setInit(int32_t init);
    void setObjectiveType(objective_type obj);
    bool comparePriorities(int64_t p1, int64_t p2, parity_comp rel=BET);
    void exportFile(game_type type, std::string filename);
    void printGame();
    void flipGame();
};

// ============================================================================

class GameView {
private:
    Game& g;
public:
    vec<bool> vs;
    vec<bool> es;
    GameView(Game& g);

    //-------------------------------------------------------------------------

    vec<int32_t> getVertices();         // Return a set of active vertices
    vec<int32_t> getEdges();            // Return a set of active edges
    vec<int32_t> getOuts(int32_t v);    // Return a set of active outs of v
    vec<int32_t> getIns(int32_t w);     // Return a set of active ins of w
    std::string viewCurrent();

    void activeAll();
    void deactiveAll();
};

#endif // GAME_H
