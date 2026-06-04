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
#ifndef WINNING_CONDITIONS_H
#define WINNING_CONDITIONS_H

#ifndef GAME_H
#include "../utils/game.h"
#endif

#include "chuffed/support/vec.h"

//=============================================================================

class WinningCondition {
protected:
    Game& g;
    parity_type playerSAT;

public:
    WinningCondition(Game& g, parity_type playerSAT=EVEN) 
    : g(g), playerSAT(playerSAT)
    {
    }
    virtual ~WinningCondition() = default;
    //-----------------------------------------------------------------------
    virtual bool satisfy(   vec<int32_t>& pathV,
                            vec<int32_t>& pathE,
                            vec<int64_t>& pathW,
                            int32_t cycleIndex ) = 0;
};

//===========================================================================

class ParityCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
public:

    bool satisfy(   vec<int32_t>& pathV,
                    vec<int32_t>& pathE,
                    vec<int64_t>& pathW,
                    int32_t cycleIndex ) override 
    {
        int64_t m = g.priors[pathV[cycleIndex]];
        for (int32_t i=cycleIndex+1; i<pathV.size(); i++) {
            if (g.isBetter(g.priors[pathV[i]],m)) {
                m = g.priors[pathV[i]];
            }
        }
        return m%2==playerSAT;
    }
};

//===========================================================================

class EnergyCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
private:
    int64_t threshold;
public:

    void setThreshold(int64_t t) { threshold = t; }
    
    bool satisfy(   vec<int32_t>& pathV,
                    vec<int32_t>& pathE,
                    vec<int64_t>& pathW,
                    int32_t cycleIndex ) override 
    {
        int64_t sum = 0;
        sum = pathW.last()-pathW[cycleIndex]+g.weights[pathE[cycleIndex]];
        
        if (playerSAT == EVEN) {
            return sum >= threshold;
        }
        return sum < threshold;
    }
};

//===========================================================================

class MeanPayoffCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
private:
    double threshold;
public:

    void setThreshold(double t) { threshold = t; }

    bool satisfy(   vec<int32_t>& pathV,
                    vec<int32_t>& pathE,
                    vec<int64_t>& pathW,
                    int32_t cycleIndex ) override 
    {
        int64_t sum = 0;
        sum = pathW.last()-pathW[cycleIndex]+g.weights[pathE[cycleIndex]];
        double avg = static_cast<double>(sum) / (pathE.size() - cycleIndex);

        if (playerSAT == EVEN) {
            return avg >= threshold;
        }
        return avg < threshold;
    }
};

//===========================================================================

class BuchiCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
private:
    vec<int32_t> B;
public:

    void pushVertexInB(int32_t v) {
        B.push(v);
    }

    bool satisfy(   vec<int32_t>& pathV,
                    vec<int32_t>& pathE,
                    vec<int64_t>& pathW,
                    int32_t cycleIndex ) override 
    {
        for (int32_t i=cycleIndex+1; i<pathV.size(); i++) {
            for (int32_t j=0; j<B.size(); j++) {
                if (pathV[i] == B[j]) {
                    return true;
                }
            }
        }
        return false;
    }
};

//===========================================================================

class CoBuchiCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
private:
    vec<int32_t> B;
public:

    void pushVertexInB(int32_t v) {
        B.push(v);
    }

    bool satisfy(   vec<int32_t>& pathV,
                    vec<int32_t>& pathE,
                    vec<int64_t>& pathW,
                    int32_t cycleIndex ) override 
    {
        for (int32_t i=cycleIndex+1; i<pathV.size(); i++) {
            for (int32_t j=0; j<B.size(); j++) {
                if (pathV[i] == B[j]) {
                    return false;
                }
            }
        }
        return true;
    }
};

//===========================================================================

struct RabinPair {
    vec<int32_t> B;
    vec<int32_t> F;
};

class RabinCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
private:
    vec<RabinPair> C;
public:

    void pushRabinPair() {
        C.push();
    }

    void updateRabinPairB(size_t i, int32_t v) {
        C[i].B.push(v);
    }
    void updateRabinPairF(size_t i, int32_t v) {
        C[i].F.push(v);
    }

    bool satisfy(   vec<int32_t>& pathV,
                    vec<int32_t>& pathE,
                    vec<int64_t>& pathW,
                    int32_t cycleIndex ) override 
    {
        for (size_t pairIdx = 0; pairIdx < C.size(); pairIdx++) {
            bool isB = false;
            bool isF = false;

            for (int32_t pathIdx = cycleIndex+1; pathIdx < pathV.size(); pathIdx++) {
                int32_t currentState = pathV[pathIdx];

                if (!isB) {
                    for (size_t j = 0; j < C[pairIdx].B.size(); j++) {
                        if (currentState == C[pairIdx].B[j]) {
                            isB = true;
                            break;
                        }
                    }
                }

                if (!isF) {
                    for (size_t j = 0; j < C[pairIdx].F.size(); j++) {
                        if (currentState == C[pairIdx].F[j]) {
                            isF = true;
                            break; // Found an F state, we can stop checking F for this pair
                        }
                    }
                }

                if (isB) break;
            }

            if (!isB && isF) return true;
        }
        
        return false;
    }


};

#endif // WINNING_CONDITIONS_H