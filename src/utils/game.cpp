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
#include <iostream>
#include <vector>
#include <fstream>
#include <regex>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <random>
#include <chrono> 

#include "game.h"
#include "gameparser.h"

//-----------------------------------------------------------------------------

parity_type opponent(parity_type PARITY) {
    if (PARITY==EVEN) return ODD; return EVEN;
}

//-----------------------------------------------------------------------------

#include <cstdio>  
#include <cstring> 
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

//-----------------------------------------------------------------------------
// Default game

Game::Game( vec<int8_t>&    owners,
            vec<int64_t>&   priors,
            vec<int32_t>&   sources,
            vec<int32_t>&   targets,
            vec<int64_t>&   weights,
            int32_t         init,
            objective_type  obj)
:   owners(owners), priors(priors), sources(sources), targets(targets), 
    weights(weights), init(init), objective(obj) 
{
    nvertices   = owners.size();
    nedges      = sources.size();

    setInit(init);
    
    outs.growTo(nvertices);
    ins .growTo(nvertices);

    for (size_t i=0; i<nedges; i++) {
        outs[sources[i]].push(i);
        ins [targets[i]].push(i);
    }
}

//-----------------------------------------------------------------------------
// Imported game from DZN or GM

Game::Game( game_type       type, 
            std::string     filename,
            int32_t         init, 
            objective_type  obj,
            int64_t         lbound,
            int64_t         ubound)
:   nvertices(0), nedges(0), init(init), objective(obj) 
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    if (!filename.empty() && filename.back() == '.') {
        switch (type) {
            case DZN:   filename.append("dzn"); break;
            case GM:    filename.append("gm");  break;
            case HOA:   filename.append("hoa"); break;
        }
    }
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file!" << std::endl;
        exit(0);
    }

    std::string line;

    try {
        switch (type) {
            case DZN:
                parseDZN(*this,file,lbound,ubound);
                setInit(init);
                break;
            case GM:
                parseGM(*this,file,lbound,ubound);
                setInit(init);
                break;
            case HOA:
                parseHOA(*this,file,lbound,ubound);
                break;
        }
    }
    catch(const std::exception& e) {
        std::string error =  "Error: Could not parse '" + filename + "'.";
        throw std::invalid_argument(error);
    }
}

//-----------------------------------------------------------------------------
// Jurdzinski/Random/Mladder game/SPRAND-Randx

Game::Game( game_type       type,
            vec<int32_t>&   vals,
            int32_t         init,
            objective_type  obj,
            int64_t         lbound,
            int64_t         ubound)
:   nvertices(0), nedges(0), init(init), objective(obj) 
{
    if (type == JURD) {
        int32_t levels  = vals[0];
        int32_t blocks  = vals[1];
        nvertices   = ((blocks*3)+1)*(levels-1) + ((blocks*2)+1);
        nedges      = (blocks*6)*(levels-1) + (blocks*4) + (blocks*2*(levels-1));

        std::mt19937 g(42);
        std::uniform_int_distribution<> rndWeight(lbound,ubound);
        int32_t es = 0;
        int32_t os = 0;
        
        for (size_t l=1; l<levels; l++) {
            os = ((blocks*3)+1)*(levels-1);
            for (size_t b=0; b<blocks; b++) {
                owners.push(1);
                owners.push(0);
                owners.push(0);
                priors.push((levels-l)*2);
                priors.push((levels-l)*2+1);
                priors.push((levels-l)*2);

                sources.push(es);   targets.push(es+1);
                weights.push(rndWeight(g));
                sources.push(es);   targets.push(es+2);
                weights.push(rndWeight(g));
                sources.push(es+1); targets.push(es+2);
                weights.push(rndWeight(g));
                sources.push(es+2); targets.push(es);
                weights.push(rndWeight(g));

                sources.push(es+2); targets.push(es+3);
                weights.push(rndWeight(g));
                sources.push(es+3); targets.push(es+2);
                weights.push(rndWeight(g));

                sources.push(es+2); targets.push(os+1);
                weights.push(rndWeight(g));
                sources.push(os+1); targets.push(es+2);
                weights.push(rndWeight(g));

                es += 3;
                os += 2;
            }
            owners.push(1);
            priors.push((levels-l)*2);
            es += 1;
        }
        int32_t l = levels;
        for (size_t b=0; b<blocks; b++) {
            owners.push(0);
            owners.push(1);

            priors.push((levels-l)*2);
            priors.push((levels-l)*2+1);

            sources.push(es);   targets.push(es+1);
            weights.push(rndWeight(g));
            sources.push(es+1); targets.push(es);
            weights.push(rndWeight(g));
            sources.push(es+1); targets.push(es+2);
            weights.push(rndWeight(g));
            sources.push(es+2); targets.push(es+1);
            weights.push(rndWeight(g));

            es += 2;
        }
        owners.push(0);
        priors.push((levels-l)*2);

        outs.growTo(nvertices);
        ins .growTo(nvertices);
        for (size_t i=0; i<nedges; i++) {
            outs[sources[i]].push(i);
            ins [targets[i]].push(i);
        }
    }
    else if (type == RAND) {
        nvertices   = vals[0];
        nedges      = 0;
    
        std::mt19937 g(42);
    
        owners.growTo(nvertices/2,0);
        owners.growTo(nvertices,1);
        std::uniform_int_distribution<> rndPositons(0, nvertices-1);
        for(size_t i=0; i<nvertices/10; i++) {
            int32_t o1 = rndPositons(g);
            int32_t o2 = rndPositons(g);
            int8_t temp = owners[o1];
            owners[o1] = owners[o2];
            owners[o2] = temp;
        }

        std::uniform_int_distribution<> rndPriors(0, vals[1]);
        std::uniform_int_distribution<> rndWeight(lbound,ubound);

        for (size_t i=0; i<nvertices; i++) {
            priors.push(rndPriors(g));
        }
    
        outs.growTo(nvertices);
        ins .growTo(nvertices);
        for (size_t v=0; v<nvertices; v++) {
            std::vector<int> ws;
            for (size_t i=0; i < nvertices; i++) { ws.push_back(i); }
            std::shuffle(ws.begin(), ws.end(), g);
            auto [lb, ub] = std::minmax(vals[2], vals[3]);
            std::uniform_int_distribution<> rndNedges(lb, ub);

            int32_t es = rndNedges(g);
            for (size_t i=0; i<es; i++) {
                sources.push(v);
                targets.push(ws[i]);
                weights.push(rndWeight(g));
                outs[v].push(nedges);
                ins[ws[i]].push(nedges);
                nedges++;
            }
        }
    }
    else if (type == MLADDER) {
        int32_t bl  = vals[0];
        nvertices   = bl*3+1;
        nedges      = bl*4+1;

        std::mt19937 g(42);
        owners.growTo(nvertices/2,0);
        owners.growTo(nvertices,1);
        std::uniform_int_distribution<> rndPositons(0, nvertices-1);
        for(size_t i=0; i<nvertices/10; i++) {
            int32_t o1 = rndPositons(g);
            int32_t o2 = rndPositons(g);
            int8_t temp = owners[o1];
            owners[o1] = owners[o2];
            owners[o2] = temp;
        }

        std::uniform_int_distribution<> rndWeight(lbound,ubound);

        priors  .growTo(nvertices);
        sources .growTo(nedges);
        targets .growTo(nedges);
        weights .growTo(nedges);
        outs    .growTo(nvertices);
        ins     .growTo(nvertices);

        int64_t consecutive = bl*2;
        priors[0] = consecutive--;
        for (size_t i=0; i<bl; i++) {
            priors[i*3+1] = 0;
            priors[i*3+2] = consecutive--;
            priors[i*3+3] = consecutive--;
        }

        int32_t e = 0;
        for (size_t i=0; i<bl; i++) {
            sources[e] = i*3+0;
            targets[e] = i*3+1;
            weights[e] = rndWeight(g);
            outs[i*3+0].push(e);
            ins [i*3+1].push(e);
            e++;

            sources[e] = i*3+1;
            targets[e] = i*3+2;
            weights[e] = rndWeight(g);
            outs[i*3+1].push(e);
            ins [i*3+2].push(e);
            e++;

            sources[e] = i*3+1;
            targets[e] = i*3+3;
            weights[e] = rndWeight(g);
            outs[i*3+1].push(e);
            ins [i*3+3].push(e);
            e++;

            sources[e] = i*3+2;
            targets[e] = i*3+3;
            weights[e] = rndWeight(g);
            outs[i*3+2].push(e);
            ins [i*3+3].push(e);
            e++;
        }

        sources[e] = bl*3;
        targets[e] = 0;
        weights[e] = rndWeight(g);
        outs[bl*3].push(e);
        ins [0].push(e);
    }
    else if (type == SPRAND) {
        nvertices       = vals[0];
        int32_t density = vals[1];
        nedges          = vals[0]*density;
    
        std::mt19937 g(42);
    
        owners.growTo(nvertices/2,0);
        owners.growTo(nvertices,1);
        std::uniform_int_distribution<> rndPositons(0, nvertices-1);
        for(size_t i=0; i<nvertices/10; i++) {
            int32_t o1 = rndPositons(g);
            int32_t o2 = rndPositons(g);
            int8_t temp = owners[o1];
            owners[o1] = owners[o2];
            owners[o2] = temp;
        }

        std::uniform_int_distribution<> rndPriors(0, nvertices-1);
        std::uniform_int_distribution<> rndWeight(lbound,ubound);
        std::uniform_int_distribution<> rndNode(0, nvertices-1);

        for (size_t i=0; i<nvertices; i++) {
            priors.push(rndPriors(g));
        }
    
        outs.growTo(nvertices);
        ins .growTo(nvertices);

        // Hamilton path
        std::vector<int32_t> cycle(nvertices);
        std::iota(cycle.begin(), cycle.end(), 0); 
        std::shuffle(cycle.begin(), cycle.end(), g);
        for (size_t i = 0; i < nvertices; i++) {
            int32_t u = cycle[i];
            int32_t v_cycle = cycle[(i + 1) % nvertices];
            uint32_t edgeIdx = sources.size();
            sources.push(u);
            targets.push(v_cycle);
            weights.push(rndWeight(g));
            outs[u].push(i);
            ins[v_cycle].push(i);

            // Creating the remaining x-1 edges
            int32_t edgesAdded = 1;
            while (edgesAdded < density) {
                int32_t v_rand = rndNode(g);

                bool isDuplicate = (v_rand == v_cycle);
                for (size_t j=0; j<outs[u].size(); j++) {
                    int32_t eIdx = outs[u][j];
                    if (targets[eIdx] == v_rand) {
                        isDuplicate = true;
                        break;
                    }
                }

                if (u != v_rand && !isDuplicate) {
                    edgeIdx = sources.size();
                    sources.push(u);
                    targets.push(v_rand);
                    weights.push(rndWeight(g));
                    outs[u].push(edgeIdx);
                    ins[v_rand].push(edgeIdx);
                    edgesAdded++;
                }
            }
        }
    }
    else if (type == SQNC) {
        int32_t size    = vals[0];
        nvertices       = size*size;    // sqrt(desired_size)
        int32_t type    = vals[1];
        nedges          = nvertices*2;  // temporary
    
        std::mt19937 g(42);
    
        owners.growTo(nvertices/2,0);
        owners.growTo(nvertices,1);
        std::uniform_int_distribution<> rndPositons(0, nvertices-1);
        for(size_t i=0; i<nvertices/10; i++) {
            int32_t o1 = rndPositons(g);
            int32_t o2 = rndPositons(g);
            int8_t temp = owners[o1];
            owners[o1] = owners[o2];
            owners[o2] = temp;
        }

        std::uniform_int_distribution<> rndPriors(0, nvertices-1);
        std::uniform_int_distribution<> rndWeightShort(1,100);
        std::uniform_int_distribution<> rndWeightLong(1000,10000);
        std::uniform_int_distribution<> rndNode(0, nvertices-1);

        for (size_t i=0; i<nvertices; i++) {
            priors.push(rndPriors(g));
        }
    
        outs.growTo(nvertices);
        ins .growTo(nvertices);

        int32_t e=0;
        for (size_t r=0; r<size; r++) {
            // Horizontal edges
            for (size_t c=0; c<size; c++) {
                int32_t v = r*size+c;
                int32_t w = c<size-1 ? r*size+c+1 : r*size;
                sources.push(v);
                targets.push(w);
                outs[v].push(e);
                ins[w].push(e);
                weights.push(rndWeightLong(g));
                e++;
            }
            // Vertical edges
            for (size_t c=0; c<size; c++) {
                int32_t v = r * size + c;
                int32_t w = (r < size - 1) ? (r + 1) * size + c : c;
                sources.push(v);
                targets.push(w);
                outs[v].push(e);
                ins[w].push(e);
                weights.push(rndWeightShort(g));
                e++;
            }            
        }

        std::uniform_int_distribution<> rndCycle(0, nvertices-1);
        if (type==2) {
            int32_t nCycles = 1;
            for (size_t c=0; c<nCycles; c++) {
                int32_t cycleLen = 3;

                int32_t v = rndCycle(g);
                int32_t f = v;
                int32_t added = 1;
                while (added < cycleLen) {
                    int32_t w = rndCycle(g);
                    
                    bool found = false;
                    for (size_t i=0; i<outs[v].size(); i++) {
                        int32_t e = outs[v][i];
                        if (targets[e]==w) {
                            found = true;
                            break;
                        }
                    }
                    if (found) continue;

                    if (added == cycleLen-1) {
                        for (size_t i=0; i<outs[w].size(); i++) {
                            int32_t e = outs[w][i];
                            if (targets[e]==f) {
                                found = true;
                                break;
                            }
                        }
                        if (found) continue;
                    }

                    sources.push(v);
                    targets.push(w);
                    outs[v].push(e);
                    ins[w].push(e);
                    weights.push(v == f ? -1 : 0);
                    e++;
                    v = w;
                    added++;
                }
                sources.push(v);
                targets.push(f);
                outs[v].push(e);
                ins[f].push(e);
                weights.push(0);
                e++;
            }
        }
        else if (type==3) {
            int32_t nCycles = size;
            for (size_t c=0; c<nCycles; c++) {
                int32_t cycleLen = 3;

                int32_t v = rndCycle(g);
                int32_t f = v;
                int32_t added = 1;
                while (added < cycleLen) {
                    int32_t w = rndCycle(g);
                    
                    bool found = false;
                    for (size_t i=0; i<outs[v].size(); i++) {
                        int32_t e = outs[v][i];
                        if (targets[e]==w) {
                            found = true;
                            break;
                        }
                    }
                    if (found) continue;

                    if (added == cycleLen-1) {
                        for (size_t i=0; i<outs[w].size(); i++) {
                            int32_t e = outs[w][i];
                            if (targets[e]==f) {
                                found = true;
                                break;
                            }
                        }
                        if (found) continue;
                    }

                    sources.push(v);
                    targets.push(w);
                    outs[v].push(e);
                    ins[w].push(e);
                    weights.push(v == f ? -1 : 0);
                    e++;
                    v = w;
                    added++;
                }
                sources.push(v);
                targets.push(f);
                outs[v].push(e);
                ins[f].push(e);
                weights.push(0);
                e++;
            }

        }
        else if (type==4) {
            int32_t nCycles = size/2;
            for (size_t c=0; c<nCycles; c++) {
                int32_t cycleLen = size;

                int32_t v = rndCycle(g);
                int32_t f = v;
                int32_t added = 1;
                while (added < cycleLen) {
                    int32_t w = rndCycle(g);
                    
                    bool found = false;
                    for (size_t i=0; i<outs[v].size(); i++) {
                        int32_t e = outs[v][i];
                        if (targets[e]==w) {
                            found = true;
                            break;
                        }
                    }
                    if (found) continue;

                    if (added == cycleLen-1) {
                        for (size_t i=0; i<outs[w].size(); i++) {
                            int32_t e = outs[w][i];
                            if (targets[e]==f) {
                                found = true;
                                break;
                            }
                        }
                        if (found) continue;
                    }

                    sources.push(v);
                    targets.push(w);
                    outs[v].push(e);
                    ins[w].push(e);
                    weights.push(v == f ? -1 : 0);
                    e++;
                    v = w;
                    added++;
                }
                sources.push(v);
                targets.push(f);
                outs[v].push(e);
                ins[f].push(e);
                weights.push(0);
                e++;
            }
        }
        else if (type==5) {
            int32_t nCycles = 1;
            for (size_t c=0; c<nCycles; c++) {
                int32_t cycleLen = nvertices;

                int32_t v = rndCycle(g);
                int32_t f = v;
                int32_t added = 1;
                while (added < cycleLen) {
                    int32_t w = rndCycle(g);
                    
                    bool found = false;
                    for (size_t i=0; i<outs[v].size(); i++) {
                        int32_t e = outs[v][i];
                        if (targets[e]==w) {
                            found = true;
                            break;
                        }
                    }
                    if (found) continue;

                    if (added == cycleLen-1) {
                        for (size_t i=0; i<outs[w].size(); i++) {
                            int32_t e = outs[w][i];
                            if (targets[e]==f) {
                                found = true;
                                break;
                            }
                        }
                        if (found) continue;
                    }

                    sources.push(v);
                    targets.push(w);
                    outs[v].push(e);
                    ins[w].push(e);
                    weights.push(v == f ? -1 : 0);
                    e++;
                    v = w;
                    added++;
                }
                sources.push(v);
                targets.push(f);
                outs[v].push(e);
                ins[f].push(e);
                weights.push(0);
                e++;
            }
        }
        nedges = e;

        // Transformation
        vec<int32_t> potentials(nvertices);
        for (int32_t i = 0; i < nvertices; i++) {
            potentials[i] = std::rand() % 16384;
        }

        for (size_t i = 0; i < nedges; i++) {
            int32_t u = sources[i];
            int32_t v = targets[i];
            weights[i] = weights[i] + potentials[u] - potentials[v];
        }

    }

    setInit(init);
}

//-----------------------------------------------------------------------------

void Game::setInit(int32_t init) {
    if (init < 0) {
        init = 0;
        std::cerr << "Warning: Initial vertex set to 0." << std::endl;
    }
    else if (init >= nvertices) {
        init = nvertices - 1;
        std::cerr   << "Warning: Initial vertex set to " << init << "." 
                    << std::endl;
    }
    this->init = init;
}

//-----------------------------------------------------------------------------

void Game::setObjectiveType(objective_type obj) {
    objective = obj;
}

//-----------------------------------------------------------------------------

bool Game::isBetter(int64_t p1,int64_t p2) {
    if (objective==MIN && p1 < p2) return true; 
    if (objective==MAX && p1 > p2) return true;
    return false;
}

//-----------------------------------------------------------------------------

void Game::exportFile(game_type type, std::string filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file!" << std::endl;
        exit(0);
    }

    switch (type) {
    case DZN:
        file << "nvertices = " << nvertices << ";" << std::endl;
        file << "owners    = ["; 
        for (size_t i=0; i<owners.size(); i++) {
            file<<(i?",":"")<<(int)owners[i];
        }
        file <<"];"<<std::endl;
        file << "priors    = ["; 
        for (size_t i=0; i<priors.size(); i++) {
            file<<(i?",":"")<<priors[i];
        }
        file <<"];"<<std::endl;
        file << "nedges    = " << nedges << ";" << std::endl;
        file << "sources   = ["; 
        for (size_t i=0; i<sources.size(); i++) {
            file<<(i?",":"")<<sources[i];
        }
        file <<"];"<<std::endl;
        file << "targets   = ["; 
        for (size_t i=0; i<targets.size(); i++) {
            file<<(i?",":"")<<targets[i];
        }
        file <<"];"<<std::endl;
        file << "weights   = ["; 
        for (size_t i=0; i<weights.size(); i++) {
            file<<(i?",":"")<<weights[i];
        }
        file <<"];"<<std::endl;
        break;

    case GM: case GMW:
        file << "parity " << (nvertices-1) << ";" << std::endl;
        for (size_t v=0; v<nvertices; v++) {
            file << v << " " << priors[v] << " " << (int)owners[v] << " ";
            for (size_t e=0; e<outs[v].size(); e++) {
                file << (e?",":"") << targets[outs[v][e]];
            }
            if (type == GMW) {
                file << " \"\" ";
                for (size_t e=0; e<outs[v].size(); e++) {
                    file << (e?",":"") << weights[outs[v][e]];
                }
            }
            file << ";" << std::endl;
        }
        break;
    
    case CHPKA:
        // chanelling
        vec<int32_t> nocq2chpka(nvertices), chpka2nocq(nvertices);
        
        size_t index = 0;
        for (size_t i = 0; i < nvertices; i++) {
            if (owners[i] == 0) {
                    nocq2chpka[i] = index;
                    chpka2nocq[index] = i;
                index++;
            }
        }

        size_t partition = index;

        for (size_t i = 0; i < nvertices; i++) {
            if (owners[i] == 1) {
                nocq2chpka[i] = index;
                chpka2nocq[index] = i;
                index++;
            }
        }

        file << nvertices << " " << nedges << " " << partition << std::endl;

        for (size_t i = 0; i < nvertices; i++) {
            int32_t u = chpka2nocq[i];
            for (size_t j=0; j<outs[u].size(); j++) {
                int32_t e = outs[u][j];
                int32_t v = targets[e];
                int32_t k = nocq2chpka[v];
                int64_t weight = weights[e];
                file << i << " " << k << " " << weight << " \"" << u << "\""
                    << std::endl;
            }
        }
        break;
    }
}

//-----------------------------------------------------------------------------

void Game::printGame() {
    std::cout << "nvertices: " << owners.size() << std::endl;
    std::cout << "owners:    {";
    for (size_t v=0; v<nvertices; v++) {
        std::cout << (v?",":"") << (int)owners[v];
    }
    std::cout << "}" << std::endl;
    std::cout << "priors:    {";
    for (size_t v=0; v<nvertices; v++) {
        std::cout << (v?",":"") << priors[v];
    }
    std::cout << "}" << std::endl;

    std::cout << "nedges:    " << sources.size() << std::endl;
    std::cout << "sources:   {";
    for (size_t e=0; e<nedges; e++) {
        std::cout << (e?",":"") << sources[e];
    }
    std::cout << "}" << std::endl;
    std::cout << "targets:   {";
    for (size_t e=0; e<nedges; e++) {
        std::cout << (e?",":"") << targets[e];
    }
    std::cout << "}" << std::endl;
    std::cout << "weights:   {";
    for (size_t e=0; e<nedges; e++) {
        std::cout << (e?",":"") << weights[e];
    }
    std::cout << "}" << std::endl;
}

//-----------------------------------------------------------------------------

void Game::flipGame() {
    for (size_t v=0; v<nvertices; v++) {
        owners[v] = 1-owners[v];
        priors[v]++;
    }
}

//=============================================================================

GameView::GameView(Game& g) : g(g) {
    vs.growTo(g.nvertices,true);
    es.growTo(g.nedges,true);
}

//-----------------------------------------------------------------------------

void GameView::getVertices(vec<int32_t>& vertices) {
    vertices.clear();
    for (size_t v=0; v<g.nvertices; v++) {
        if (vs[v]) vertices.push(v);
    }
}

//-----------------------------------------------------------------------------

void GameView::getEdges(vec<int32_t>& edges){
    edges.clear();
    for (size_t e=0; e<g.nedges; e++) {
        if (es[e]) edges.push(e);
    }
}

//-----------------------------------------------------------------------------

void GameView::activeAll() {
    for (size_t v=0; v<g.nvertices; v++)   vs[v] = true;
    for (size_t e=0; e<g.nedges; e++)      es[e] = true;
}

//-----------------------------------------------------------------------------

void GameView::deactiveAll() {
    for (size_t v=0; v<g.nvertices; v++)   vs[v] = false;
    for (size_t e=0; e<g.nedges; e++)      es[e] = false;
}

//-----------------------------------------------------------------------------

void GameView::getOuts(vec<int32_t>& edges, int32_t v) {
    edges.clear();
    if (!vs[v]) return;
    for (size_t i=0; i<g.outs[v].size(); i++) {
        int32_t e = g.outs[v][i];
        int32_t w = g.targets[e];
        if (es[e] && vs[w]) edges.push(e);
    }
}

//-----------------------------------------------------------------------------

void GameView::getIns(vec<int32_t>& edges,int32_t w) {
    edges.clear();
    if (!vs[w]) return;
    for (size_t i=0; i<g.ins[w].size(); i++) {
        int32_t e = g.ins[w][i];
        int32_t u = g.sources[e];
        if (es[e] && vs[u]) edges.push(e);
    }
}

//----------------------------------------------------------------------------

std::string GameView::viewCurrent() {
    std::stringstream ss;
    ss << "{";
    for (size_t i=0; i<g.nvertices; i++) if (vs[i]) {
        if (i>0) ss << ",";
        ss << i << ",";
    }
    ss << "} {";
    for (size_t i=0; i<g.nedges; i++) if (es[i]) {
        if (i>0) ss << ",";
        ss << i << ",";
    }
    ss << "}";

    return ss.str();
}
