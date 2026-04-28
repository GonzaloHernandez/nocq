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
    virtual bool satisfy(   vec<int32_t>& pathV,vec<int32_t>& pathE,
                            vec<int64_t>& pathW,
                            int32_t cycleIndex) = 0;
};

//===========================================================================

class ParityCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
public:

    bool satisfy(   vec<int32_t>& pathV,vec<int32_t>& pathE,
                    vec<int64_t>& pathW,
                    int32_t cycleIndex) override 
    {
        int64_t m = g.priors[pathV[cycleIndex]];
        for (int32_t i=cycleIndex+1; i<pathV.size(); i++) {
            if (g.isBetter(g.priors[pathV[i]],m)) {
                m = g.priors[pathV[i]];
            }
        }
        return m%2==playerSAT;
    };
};

//===========================================================================

class EnergyCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
private:
    int64_t threshold;
public:

    void setThreshold(int64_t t) { threshold = t; }
    
    bool satisfy(   vec<int32_t>& pathV,vec<int32_t>& pathE,
                    vec<int64_t>& pathW,
                    int32_t cycleIndex) override 
    {
        int64_t sum = 0;
        // for (int32_t i=cycleIndex; i<pathE.size(); i++) {
        //     sum += g.weights[pathE[i]];
        // }
        sum = pathW.last()-pathW[cycleIndex]+g.weights[pathE[cycleIndex]];
        
        if (playerSAT == EVEN) {
            return sum >= threshold;
        }
        return sum < threshold;
    };
};

//===========================================================================

class MeanPayoffCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
private:
    float threshold;
public:

    void setThreshold(float t) { threshold = t; }

    bool satisfy(   vec<int32_t>& pathV,vec<int32_t>& pathE,
                    vec<int64_t>& pathW,
                    int32_t cycleIndex) override 
    {
        float sum = 0;
        for (int32_t i=cycleIndex; i<pathE.size(); i++) {
            sum += g.weights[pathE[i]];
        }
        float avg = sum / (float)(pathE.size() - cycleIndex);

        if (playerSAT == EVEN) {
            return avg >= threshold;
        }
        return avg < threshold;
    };
};

#endif // WINNING_CONDITIONS_H