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
#include <string>
#include <sstream>
#include <vector>

#include "game.h"

//-----------------------------------------------------------------------------

void parseline_dzn(const std::string& line, vec<int8_t>& myvec) {
    size_t start = line.find('[');
    size_t end = line.find(']');
    
    if (start != std::string::npos && end != std::string::npos && end > start){
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

void parseline_dzn(const std::string& line, vec<int32_t>& myvec) {
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

void parseline_dzn(const std::string& line, vec<int64_t>& myvec) {
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

void parseline_dzn(const std::string& line, vec<float>& myvec) {
    size_t start = line.find('[');
    size_t end = line.find(']');
    
    if (start != std::string::npos && end != std::string::npos && end > start) {
        std::string values = line.substr(start + 1, end - start - 1);
        std::stringstream ss(values);
        std::string segment;

        while (std::getline(ss, segment, ',')) {
            try {
                segment.erase(0, segment.find_first_not_of(" \t\r\n"));
                segment.erase(segment.find_last_not_of(" \t\r\n") + 1);

                if (segment.empty()) continue;

                size_t slashPos = segment.find('/');
                if (slashPos != std::string::npos) {
                    // It's a fraction: split into numerator and denominator
                    float num = std::stof(segment.substr(0, slashPos));
                    float den = std::stof(segment.substr(slashPos + 1));
                    
                    if (den != 0.0f) {
                        myvec.push(num / den);
                    } else {
                        // Handle division by zero if necessary
                    }
                } else {
                    // It's a standard float
                    myvec.push(std::stof(segment));
                }
            } catch (...) {
                // Skip values that aren't numbers (like extra spaces)
            }
        }
    }
}

//---------------------------------------------------------------------------

size_t skip_whitespace(const std::string& line, size_t init) {
    while (init < line.size() && std::isspace(line[init])) {
        init++;
    }
    return init;
}

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

bool parseline_gm(const std::string&  line, 
                        int32_t&            vId,
                        int64_t&            vPriority,
                        int8_t&             vOwner,
                        vec<int32_t>&       vOuts,
                        std::string&        vComment,
                        vec<int64_t>&       oWeights)
{
    vOuts.clear();
    oWeights.clear();
    vComment.clear();

    size_t current = 0;

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
        current = comment_end+1;
    }

    // --- Extract Weights (Second CSV block) ---
    current = skip_whitespace(line, current);
    parse_csv_block(oWeights);
    return true;
}

//-----------------------------------------------------------------------------

void fixZeros(Game& g) {
    for (size_t i=0; i<g.sources.size(); i++) {
        g.sources[i]--;
        g.targets[i]--;
    }
}

//-----------------------------------------------------------------------------

void parseDZN(Game& g, std::ifstream& file, int64_t lbound, int64_t ubound) {
    std::string line;

    while (getline(file, line)) {
        if (line.find("nvertices") != std::string::npos) {
            g.nvertices = stoi(line.substr(line.find("=") + 1));
        } else if (line.find("nedges") != std::string::npos) {
            g.nedges = stoi(line.substr(line.find("=") + 1));
        } else if (line.find("owners") != std::string::npos) {
            parseline_dzn(line,g.owners);
        } else if (line.find("priors") != std::string::npos) {
            parseline_dzn(line,g.priors);
        } else if (line.find("sources") != std::string::npos) {
            parseline_dzn(line,g.sources);
        } else if (line.find("targets") != std::string::npos) {
            parseline_dzn(line,g.targets);
        } else if (line.find("weights") != std::string::npos) {
            parseline_dzn(line,g.weights);
        }
    }
    file.close();

    if (g.nvertices < 1 || g.nedges < 1 || 
        g.owners.size() < g.nvertices || g.priors.size() < g.nvertices ||
        g.sources.size() < g.nedges || g.targets.size() < g.nedges)
    {
        throw std::invalid_argument("");
    }

    bool hasZeros = false;
    for (size_t e=0; e<g.nedges; e++) {
        if (g.sources[e]==0) { hasZeros = true; break; }
    }
    if (!hasZeros) fixZeros(g);
    g.outs.growTo(g.nvertices);
    g.ins .growTo(g.nvertices);
    for(int32_t i=0; i<g.nedges; i++) {
        g.outs[g.sources[i]].push(i);
        g.ins [g.targets[i]].push(i);
    }

    if (g.weights.size()==0) {
        std::random_device rd;
        std::mt19937 rand(rd());
        std::uniform_int_distribution<> rndWeight(lbound, ubound);
        for (size_t i=0; i< g.nedges; i++) {
            if (lbound == ubound) {
                g.weights.push(lbound);
            } else {
                g.weights.push(rndWeight(rand));
            }
        }
    }
}

//-----------------------------------------------------------------------------

void parseGM(Game& g, std::ifstream& file, int64_t lbound, int64_t ubound) {
    std::string line;

    int32_t lastvertex = 0;
    vec<int32_t>        tverts;
    vec<vec<int32_t>>   tedges;
    vec<vec<int64_t>>     tweights;
    int32_t counter = 0;

    std::random_device rd;
    std::mt19937 rand(rd());
    std::uniform_int_distribution<> rndWeight(lbound, ubound);

    while (getline(file, line)) {
        if (line.empty()) continue;
        if (line.find("parity") != std::string::npos) {
            lastvertex = stoi(line.substr(line.find(" ")));
            tverts.growTo(lastvertex + 1);
        } else if (line.find("init") != std::string::npos) {
            g.init = stoi(line.substr(line.find(" ")));
        } else {
            int32_t         vId;
            int64_t         vPriority;
            int8_t          vOwner;
            vec<int32_t>    vOuts;
            std::string     vComment;
            vec<int64_t>      oWeights;
            
            bool ok = parseline_gm( line, vId, vPriority, vOwner,
                                    vOuts, vComment, oWeights );

            if (!ok) continue;

            if ((oWeights.size() < vOuts.size())) {
                size_t missing;
                missing = vOuts.size() - oWeights.size();
                
                for (size_t i = 0; i < missing; ++i) {
                    if (lbound == ubound) {
                        oWeights.push(lbound);
                    } else {
                        oWeights.push(rndWeight(rand));
                    }
                }
            }
            else if (oWeights.size() > g.outs.size()) {
                oWeights.growTo(g.outs.size());
            }

            g.owners.push(vOwner);
            g.priors.push(vPriority);
            tedges.push();
            tweights.push();
            tverts[vId] = counter;
            for(size_t i=0; i<vOuts.size(); i++) {
                tedges.last().push(vOuts[i]);
                tweights.last().push(oWeights[i]);
            }
            
            counter++;
        }
    }
    file.close();

    g.nvertices = counter;
    g.outs.growTo(g.nvertices);
    g.ins.growTo(g.nvertices);

    g.nedges = 0;
    for (size_t v = 0; v < g.nvertices; v++) {
        for (size_t t = 0; t < tedges[v].size(); t++) {
            int32_t w = tverts[tedges[v][t]];
            
            g.sources.push(v);
            g.targets.push(w);
            g.weights.push(tweights[v][t]);                
            g.outs[v].push(g.nedges);
            g.ins[w].push(g.nedges);
            g.nedges++;
        }
    }

    if (g.nvertices < 1 || g.nedges < 1 || 
        g.owners.size() < g.nvertices || g.priors.size() < g.nvertices ||
        g.sources.size() < g.nedges || g.targets.size() < g.nedges)
    {
        throw std::invalid_argument("");
    }

}

//-----------------------------------------------------------------------------

void parseHOA(Game& g, std::ifstream& file, int64_t lbound, int64_t ubound) {
    std::string line;

    int32_t current_vId = -1;
    int64_t current_priority = 0;
    bool inside_body = false;
    int32_t max_controllable_ap = -1;
    vec<bool> is_controllable;

    std::random_device rd;
    std::mt19937 rand(rd());
    std::uniform_int_distribution<> rndWeight(lbound, ubound);

    while (getline(file, line)) {
        if (line.empty()) continue;
        size_t first_non_space = line.find_first_not_of(" \t\r\n");
        if (first_non_space == std::string::npos) continue;
        line = line.substr(first_non_space);

        if (!inside_body) {
            if (line.find("--BODY--") != std::string::npos) {
                inside_body = true;
                g.owners.growTo(g.nvertices);
                g.priors.growTo(g.nvertices);
                g.outs.growTo(g.nvertices);
                g.ins.growTo(g.nvertices);
            }
            else if (line.find("States:") != std::string::npos) {
                g.nvertices = stoi(line.substr(line.find(":") + 1));
            }
            else if (line.find("Start:") != std::string::npos) {
                g.init = stoi(line.substr(line.find(":") + 1));
            }
            else if (line.find("acc-name:") != std::string::npos) {
                if (line.find("parity") == std::string::npos ||
                    line.find("max") == std::string::npos ||
                    line.find("even") == std::string::npos) {
                    throw std::invalid_argument("Error: HOA file must be 'parity max even'.");
                }
            }
            else if (line.find("AP:") == 0) {
                int32_t total_aps = stoi(line.substr(line.find(":") + 1));
                is_controllable.growTo(total_aps);
                for (int32_t i = 0; i < total_aps; i++) is_controllable[i] = false;
            }
            else if (line.find("controllable-AP:") != std::string::npos) {
                std::stringstream ss(line.substr(line.find(":") + 1));
                int32_t ap_idx;
                while (ss >> ap_idx) {
                    if (ap_idx < is_controllable.size()) {
                        is_controllable[ap_idx] = true;
                    }
                }
            }
            else if (line.find("properties:") != std::string::npos) {
                if (line.find("deterministic") == std::string::npos ||
                    line.find("complete") == std::string::npos ||
                    line.find("colored") == std::string::npos) {
                    throw std::invalid_argument("Error: HOA file must be 'deterministic complete colored'.");
                }
            }
            continue;
        }

        if (line.find("--END--") != std::string::npos) break;

        if (line.find("State:") == 0) {
            std::stringstream ss(line);
            std::string dummy;
            ss >> dummy >> current_vId;

            size_t brace_open = line.find('{');
            size_t brace_close = line.find('}');
            if (brace_open != std::string::npos && brace_close != std::string::npos) {
                current_priority = stoll(line.substr(brace_open + 1, brace_close - brace_open - 1));
            } else {
                current_priority = 0;
            }
            g.priors[current_vId] = current_priority;
            continue;
        }

        size_t guard_close = line.find(']');
        if (guard_close != std::string::npos) {
            std::string guard_str = line.substr(0, guard_close + 1);
            int32_t target_vId = stoi(line.substr(guard_close + 1));

            if (guard_str != "[t]") {
                size_t first_digit_pos = guard_str.find_first_of("0123456789");
                if (first_digit_pos != std::string::npos) {
                    int32_t first_ap = stoi(guard_str.substr(first_digit_pos));

                    // Look up directly if Player 0 owns this AP
                    if (first_ap < is_controllable.size() && is_controllable[first_ap]) {
                        g.owners[current_vId] = 0; // Player 0
                    } else {
                        g.owners[current_vId] = 1; // Player 1
                    }
                }
            } else {
                g.owners[current_vId] = 0; 
            }

            g.sources.push(current_vId);
            g.targets.push(target_vId);
            g.outs[current_vId].push(g.nedges);
            g.ins[target_vId].push(g.nedges);
            g.weights.push((lbound == ubound) ? lbound : rndWeight(rand));
            g.nedges++;
        }
    }

}
