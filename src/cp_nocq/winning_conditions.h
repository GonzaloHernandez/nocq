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
    virtual bool satisfy(vec<int>& pathV,vec<int>& pathE,int cycleIndex) = 0;
};

//===========================================================================

class ParityCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
public:

    bool satisfy(vec<int>& pathV,vec<int>& pathE,int cycleIndex) override {
        int m = g.priors[pathV[cycleIndex]];
        for (int i=cycleIndex+1; i<pathV.size(); i++) {
            if (g.comparePriorities(g.priors[pathV[i]],m)) {
                m = g.priors[pathV[i]];
            }
        }
        return m%2==playerSAT;
    };
};

//===========================================================================

class EnergyCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
public:

    bool satisfy(vec<int>& pathV,vec<int>& pathE,int cycleIndex) override {
        int sum = 0;
        for (int i=cycleIndex; i<pathE.size(); i++) {
            sum += g.weights[pathE[i]];
        }
        
        if (playerSAT == EVEN) {
            return sum >= 0;
        }
        return sum < 0;
    };
};

//===========================================================================

class MeanPayoffCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
private:
    int threshold;
public:

    void setThreshold(int t) { threshold = t; }

    bool satisfy(vec<int>& pathV,vec<int>& pathE,int cycleIndex) override {
        int sum = 0;
        for (int i=cycleIndex; i<pathE.size(); i++) {
            sum += g.weights[pathE[i]];
        }
        double avg = (double)sum / (double)(pathE.size() - cycleIndex);

        if (playerSAT == EVEN) {
            return avg >= threshold;
        }
        return avg < threshold;
    };
};

#endif // WINNING_CONDITIONS_H