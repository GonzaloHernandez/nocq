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

//-----------------------------------------------------------------------------

parity_type opponent(parity_type PARITY) {
    if (PARITY==EVEN) return ODD; return EVEN;
}

//-----------------------------------------------------------------------------

void Game::fixZeros() {
    for (size_t i=0; i<sources.size(); i++) {
        sources[i]--;
        targets[i]--;
    }
}

//-----------------------------------------------------------------------------

void Game::parseline_dzn(const std::string& line, vec<int8_t>& myvec) {
    size_t start = line.find('[');
    size_t end = line.find(']');
    
    if (start != std::string::npos && end != std::string::npos && end > start) {
        std::string values = line.substr(start + 1, end - start - 1);
        std::stringstream ss(values);
        std::string value;

        while (std::getline(ss, value, ',')) {
            try {
                if (!value.empty()) {
                    myvec.push(std::stoi(value));
                }
            } catch (...) {
                // Skip values that aren't numbers (like extra spaces)
            }
        }
    }
}

//-----------------------------------------------------------------------------

void Game::parseline_dzn(const std::string& line, vec<int32_t>& myvec) {
    size_t start = line.find('[');
    size_t end = line.find(']');
    
    if (start != std::string::npos && end != std::string::npos && end > start) {
        std::string values = line.substr(start + 1, end - start - 1);
        std::stringstream ss(values);
        std::string value;

        while (std::getline(ss, value, ',')) {
            try {
                if (!value.empty()) {
                    myvec.push(std::stoi(value));
                }
            } catch (...) {
                // Skip values that aren't numbers (like extra spaces)
            }
        }
    }
}

//-----------------------------------------------------------------------------

void Game::parseline_dzn(const std::string& line, vec<int64_t>& myvec) {
    size_t start = line.find('[');
    size_t end = line.find(']');
    
    if (start != std::string::npos && end != std::string::npos && end > start) {
        std::string values = line.substr(start + 1, end - start - 1);
        std::stringstream ss(values);
        std::string value;

        while (std::getline(ss, value, ',')) {
            try {
                if (!value.empty()) {
                    myvec.push(std::stoll(value));
                }
            } catch (...) {
                // Skip values that aren't numbers (like extra spaces)
            }
        }
    }
}

//---------------------------------------------------------------------------

void Game::parseline_dzn(const std::string& line, vec<float>& myvec) {
    size_t start = line.find('[');
    size_t end = line.find(']');
    
    if (start != std::string::npos && end != std::string::npos && end > start) {
        std::string values = line.substr(start + 1, end - start - 1);
        std::stringstream ss(values);
        std::string value;

        while (std::getline(ss, value, ',')) {
            try {
                if (!value.empty()) {
                    myvec.push(std::stof(value));
                }
            } catch (...) {
                // Skip values that aren't numbers (like extra spaces)
            }
        }
    }
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

size_t skip_whitespace(const std::string& line, size_t init) {
    while (init < line.size() && std::isspace(line[init])) {
        init++;
    }
    return init;
}

size_t find_token_end(const std::string& line, size_t init, char delimiter) {
    size_t end = line.find(delimiter, init);
    if (end == std::string::npos) {
        end = line.size();
    }
    size_t non_space_end = init;
    for (size_t i = init; i < end; ++i) {
        if (!std::isspace(line[i])) {
            non_space_end = i + 1;
        }
    }
    return non_space_end;
}

bool Game::parseline_gm(const std::string&  line, 
                        int32_t             vId,
                        int64_t             vPriority,
                        int8_t              vOwner,
                        vec<int32_t>&       vOuts,
                        std::string&        vComment,
                        vec<float>&         outsWeights)
{
    vOuts.clear();
    outsWeights.clear();
    vComment.clear();

    size_t current = 0;
    
    current = skip_whitespace(line, current);
    if (current >= line.size()) return false;
    size_t next = find_token_end(line, current, ' ');
    vId = std::stoi(line.substr(current, next - current));
    current = next;

    current = skip_whitespace(line, current);
    if (current >= line.size()) return false;
    next = find_token_end(line, current, ' ');
    vPriority = std::stoll(line.substr(current, next - current));
    current = next;

    current = skip_whitespace(line, current);
    if (current >= line.size()) return false;
    next = find_token_end(line, current, ' ');
    vOwner = std::stoi(line.substr(current, next - current));
    current = next;

    // --- Helper Lambda for Comma-Separated Lists ---
    auto parse_csv_block = [&](auto& target_vec) {
        current = skip_whitespace(line, current);
        size_t end_of_block = line.find_first_of(" ;\"", current);
        if (end_of_block == std::string::npos) end_of_block = line.size();

        std::string block = line.substr(current, end_of_block - current);
        std::stringstream ss(block);
        std::string item;
        
        // Identify the internal type T (int8_t, int32_t, int64_t, or float)
        using T = std::decay_t<decltype(target_vec[0])>;

        while (std::getline(ss, item, ',')) {
            if (!item.empty()) {
                if constexpr (std::is_floating_point_v<T>) {
                    target_vec.push(std::stof(item)); // Chuffed vec uses .push()
                } else {
                    // stoll is safe for all integer types; we cast it to T
                    target_vec.push(static_cast<T>(std::stoll(item)));
                }
            }
        }
        current = end_of_block;
    };

    // --- Extract Target Edges (First CSV block) ---
    std::vector<int32_t> temp_targets;
    parse_csv_block(vOuts);
    // for(auto t : temp_targets) outs.push(t);

    current = skip_whitespace(line, current);
    if (current < line.size() && line[current] == '"') {
        current++;
        size_t comment_end = line.find('"', current);
        if (comment_end != std::string::npos) {
            vComment = line.substr(current, comment_end - current);
        }
    }

    // --- Extract Weights (Second CSV block) ---
    current = skip_whitespace(line, current);
    parse_csv_block(outsWeights);
    return true;
}

//---------------------------------------------------------------------------
// Default game

Game::Game( vec<int8_t>&    owners,
            vec<int64_t>&   priors,
            vec<int32_t>&   sources,
            vec<int32_t>&   targets,
            vec<float>&     weights,
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
            float           lbound,
            float           ubound)
:   nvertices(0), nedges(0), init(init), objective(obj) 
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    if (!filename.empty() && filename.back() == '.') {
        switch (type) {
            case DZN:   filename.append("dzn"); break;
            case GM:    filename.append("gm");  break;
        }
    }
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file!" << std::endl;
        exit(0);
    }

    std::string line;

    if (type == DZN) {
        while (getline(file, line)) {
            if (line.find("nvertices") != std::string::npos) {
                nvertices = stoi(line.substr(line.find("=") + 1));
            } else if (line.find("nedges") != std::string::npos) {
                nedges = stoi(line.substr(line.find("=") + 1));
            } else if (line.find("owners") != std::string::npos) {
                parseline_dzn(line,owners);
            } else if (line.find("priors") != std::string::npos) {
                parseline_dzn(line,priors);
            } else if (line.find("sources") != std::string::npos) {
                parseline_dzn(line,sources);
            } else if (line.find("targets") != std::string::npos) {
                parseline_dzn(line,targets);
            } else if (line.find("weights") != std::string::npos) {
                parseline_dzn(line,weights);
            }
        }
        file.close();

        for (size_t e=0; e<nedges; e++) {
            if (sources[e]==0) { fixZeros(); break; }
        }
        outs.growTo(nvertices);
        ins .growTo(nvertices);
        for(int32_t i=0; i<nedges; i++) {
            outs[sources[i]].push(i);
            ins [targets[i]].push(i);
        }
    }
    else if (type == GM) {
        int32_t lastvertex = 0;
        vec<int32_t>        verts;
        vec<vec<int32_t>>   tedges;
        vec<vec<int64_t>>   tweights;
        int32_t counter = 0;

        std::random_device rd;
        std::mt19937 g(rd());
        std::uniform_real_distribution<> rndweight(lbound, ubound);

        while (getline(file, line)) {
            if (line.empty()) continue;
            if (line.find("parity") != std::string::npos) {
                lastvertex = stoi(line.substr(line.find(" ")));
                verts.growTo(lastvertex + 1);
            } else if (line.find("init") != std::string::npos) {
                init = stoi(line.substr(line.find(" ")));
            } else {
                int32_t         vId;
                int64_t         vPriority;
                int8_t          vOwner;
                vec<int32_t>    vOuts;
                std::string     vComment;
                vec<float>      outsWeights;
                
                bool ok = parseline_gm( line, vId, vPriority, vOwner,
                                        vOuts, vComment, outsWeights );

                if (!ok) continue;

                if ((outsWeights.size() < vOuts.size())) {
                    size_t missing;
                    missing = vOuts.size() - outsWeights.size();
                    
                    for (size_t i = 0; i < missing; ++i) {
                        if (lbound == ubound) {
                            outsWeights.push(lbound);
                        } else {
                            outsWeights.push(rndweight(g));
                        }
                    }
                }
                else if (outsWeights.size() > outs.size()) {
                    outsWeights.growTo(outs.size());
                }

                owners.push(vOwner);
                priors.push(vPriority);

                verts[vId] = counter;
                for(size_t i=0; i<vOuts.size(); i++) {
                    tedges.push(vOuts[i]);
                    tweights.push(outsWeights[i]);
                }
                
                counter++;
            }
        }
        file.close();

        nvertices = counter;
        outs.growTo(nvertices);
        ins.growTo(nvertices);

        nedges = 0;
        for (size_t v = 0; v < nvertices; v++) {
            for (size_t t = 0; t < tedges[v].size(); t++) {
                int32_t w = verts[tedges[v][t]];
                
                sources.push(v);
                targets.push(w);
                weights.push(tweights[v][t]);                
                outs[v].push(nedges);
                ins[w].push(nedges);
                nedges++;
            }
        }
    }

    setInit(init);
}

//-----------------------------------------------------------------------------
// Jurdzinski/Random/Mladder game

Game::Game( game_type       type,
            vec<int32_t>&   vals,
            int32_t         init,
            objective_type  obj,
            float           lbound,
            float           ubound)
:   nvertices(0), nedges(0), init(init), objective(obj) 
{
    if (type == JURD) {
        int32_t levels  = vals[0];
        int32_t blocks  = vals[1];
        nvertices   = ((blocks*3)+1)*(levels-1) + ((blocks*2)+1);
        nedges      = (blocks*6)*(levels-1) + (blocks*4) + (blocks*2*(levels-1));

        std::random_device rd;
        std::mt19937 g(rd());
        std::uniform_real_distribution<> rndweight(lbound,ubound);
        int32_t es = 1;
        int32_t os = 0;
        
        for (size_t l=1; l<levels; l++) {
            os = ((blocks*3)+1)*(levels-1)+1;
            for (size_t b=0; b<blocks; b++) {
                owners.push(1);
                owners.push(0);
                owners.push(0);
                priors.push((levels-l)*2);
                priors.push((levels-l)*2+1);
                priors.push((levels-l)*2);

                sources.push(es);   targets.push(es+1);
                weights.push(rndweight(g));
                sources.push(es);   targets.push(es+2);
                weights.push(rndweight(g));
                sources.push(es+1); targets.push(es+2);
                weights.push(rndweight(g));
                sources.push(es+2); targets.push(es);
                weights.push(rndweight(g));

                sources.push(es+2); targets.push(es+3);
                weights.push(rndweight(g));
                sources.push(es+3); targets.push(es+2);
                weights.push(rndweight(g));

                sources.push(es+2); targets.push(os+1);
                weights.push(rndweight(g));
                sources.push(os+1); targets.push(es+2);
                weights.push(rndweight(g));

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
            weights.push(rndweight(g));
            sources.push(es+1); targets.push(es);
            weights.push(rndweight(g));
            sources.push(es+1); targets.push(es+2);
            weights.push(rndweight(g));
            sources.push(es+2); targets.push(es+1);
            weights.push(rndweight(g));

            es += 2;
        }
        owners.push(0);
        priors.push((levels-l)*2);

        fixZeros();
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
    
        std::random_device rd;
        std::mt19937 g(rd());
    
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
        std::uniform_real_distribution<> rndweight(lbound,ubound);

        for (size_t i=0; i<nvertices; i++) {
            priors.push(rndPriors(g));
        }
    
        outs.growTo(nvertices);
        ins .growTo(nvertices);
        for (size_t v=0; v<nvertices; v++) {
            std::vector<int> ws;
            for (size_t i=0; i < nvertices; i++) { ws.push_back(i); }
            std::shuffle(ws.begin(), ws.end(), g);
            std::uniform_int_distribution<> rndNedges(vals[2], vals[3]);

            int32_t es = rndNedges(g);
            for (size_t i=0; i<es; i++) {
                sources.push(v);
                targets.push(ws[i]);
                weights.push(rndweight(g));
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

        std::random_device rd;
        std::mt19937 g(rd());
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

        std::uniform_real_distribution<> rndweight(lbound,ubound);

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
            weights[e] = rndweight(g);
            outs[i*3+0].push(e);
            ins [i*3+1].push(e);
            e++;

            sources[e] = i*3+1;
            targets[e] = i*3+2;
            weights[e] = rndweight(g);
            outs[i*3+1].push(e);
            ins [i*3+2].push(e);
            e++;

            sources[e] = i*3+1;
            targets[e] = i*3+3;
            weights[e] = rndweight(g);
            outs[i*3+1].push(e);
            ins [i*3+3].push(e);
            e++;

            sources[e] = i*3+2;
            targets[e] = i*3+3;
            weights[e] = rndweight(g);
            outs[i*3+2].push(e);
            ins [i*3+3].push(e);
            e++;
        }

        sources[e] = bl*3;
        targets[e] = 0;
        weights[e] = rndweight(g);
        outs[bl*3].push(e);
        ins [0].push(e);
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
            file<<(i?",":"")<<owners[i];
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
            file<<(i?",":"")<<sources[i]+1;
        }
        file <<"];"<<std::endl;
        file << "targets   = ["; 
        for (size_t i=0; i<targets.size(); i++) {
            file<<(i?",":"")<<targets[i]+1;
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
            file << v << " " << priors[v] << " " << owners[v] << " ";
            for (size_t e=0; e<outs[v].size(); e++) {
                file << (e?",":"") << targets[outs[v][e]];
            }
            if (type == GMW) {
                file << " ";
                for (size_t e=0; e<outs[v].size(); e++) {
                    file << (e?",":"") << weights[outs[v][e]];
                }
            }
            file << ";" << std::endl;
        }
        break;
    }
}

//-----------------------------------------------------------------------------

void Game::printGame() {
    std::cout << "nvertices: " << owners.size() << std::endl;
    std::cout << "owners:    {";
    for (size_t v=0; v<nvertices; v++) {
        std::cout<<owners[v]<<(v<owners.size()-1?",":"");
    }
    std::cout << "}" << std::endl;
    std::cout << "priors:    {";
    for (size_t v=0; v<nvertices; v++) {
        std::cout<<priors[v]<<(v<priors.size()-1?",":"");
    }
    std::cout << "}" << std::endl;

    std::cout << "nedges:    " << sources.size() << std::endl;
    std::cout << "sources:   {";
    for (size_t e=0; e<nedges; e++) {
        std::cout<<sources[e]<<(e<sources.size()-1?",":"");
    }
    std::cout << "}" << std::endl;
    std::cout << "targets:   {";
    for (size_t e=0; e<nedges; e++) {
        std::cout<<targets[e]<<(e<targets.size()-1?",":"");
    }
    std::cout << "}" << std::endl;
    std::cout << "weights:   {";
    for (size_t e=0; e<nedges; e++) {
        std::cout<<weights[e]<<(e<weights.size()-1?",":"");
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

void GameView::getVertices(vec<int32_t>&vs) {
    for (size_t v=0; v<g.nvertices; v++) {
        if (vs[v]) vs.push(v);
    }
}

//-----------------------------------------------------------------------------

void GameView::getEdges(vec<int32_t>& es){
    for (size_t e=0; e<g.nedges; e++) {
        if (es[e]) es.push(e);
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

void GameView::getOuts(vec<int32_t>& es, int32_t v) {
    for (size_t i=0; i<g.outs[v].size(); i++) {
        int32_t e = g.outs[v][i];
        int32_t w = g.targets[e];
        if (es[e] && vs[w]) es.push(e);
    }
}

//-----------------------------------------------------------------------------

void GameView::getIns(vec<int32_t>& es,int32_t w) {
    for (size_t i=0; i<g.ins[w].size(); i++) {
        int32_t e = g.ins[w][i];
        int32_t w = g.sources[e];
        if (es[e] && vs[w]) es.push(e);
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
