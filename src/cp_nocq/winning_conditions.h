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

struct range {
  int32_t first, last;
};

//=============================================================================

class WinningCondition {
protected:
  Game &g;
  parity_type playerSAT;

public:
  WinningCondition(Game &g, parity_type playerSAT = EVEN)
      : g(g), playerSAT(playerSAT) {}
  virtual ~WinningCondition() = default;
  //-----------------------------------------------------------------------
  virtual bool satisfy(vec<int32_t> &pathV, vec<int32_t> &pathE,
                       vec<int64_t> &pathW, int32_t cycleIndex) {
    return true;
  }
  //-----------------------------------------------------------------------
  virtual bool satisfySCC(vec<int32_t> pathV, vec<int32_t> &S) { return true; }
};

//===========================================================================

class ReachCondition : public WinningCondition {
  using WinningCondition::WinningCondition;

private:
  vec<range> T;

public:
  void pushVertexInT(int32_t v1, int32_t v2) { T.push({v1, v2}); }
  //-----------------------------------------------------------------------
  const vec<range> &getT() const { return T; }
  //-----------------------------------------------------------------------
  bool satisfy(vec<int32_t> &pathV, vec<int32_t> &pathE, vec<int64_t> &pathW,
               int32_t cycleIndex) override {
    if (playerSAT == EVEN) {
      for (int32_t i = 0; i < pathV.size(); i++) {
        for (int32_t j = 0; j < T.size(); j++) {
          if (pathV[i] >= T[j].first && pathV[i] <= T[j].last) {
            return true;
          }
        }
      }
      return false;
    } else {
      for (int32_t i = 0; i < pathV.size(); i++) {
        for (int32_t j = 0; j < T.size(); j++) {
          if (pathV[i] >= T[j].first && pathV[i] <= T[j].last) {
            return false;
          }
        }
      }
      return true;
    }
  }
  //-----------------------------------------------------------------------
  bool satisfySCC(vec<int32_t> pathV, vec<int32_t> &S) override {
    if (playerSAT == EVEN) {
      for (int32_t i = 0; i < pathV.size(); i++) {
        for (int32_t j = 0; j < T.size(); j++) {
          if (pathV[i] >= T[j].first && pathV[i] <= T[j].last) {
            S.push(pathV[i]);
            break;
          }
        }
      }
      return S.size() > 0;
    } else {
      for (int32_t i = 0; i < pathV.size(); i++) {
        bool in_T = false;
        for (int32_t j = 0; j < T.size(); j++) {
          if (pathV[i] >= T[j].first && pathV[i] <= T[j].last) {
            in_T = true;
            break;
          }
        }
        if (!in_T) {
          S.push(pathV[i]);
        }
      }
      return S.size() > 0;
    }
  }
};

//===========================================================================

class SafetyCondition : public WinningCondition {
  using WinningCondition::WinningCondition;

private:
  vec<range> U;

public:
  void pushVertexInU(int32_t v1, int32_t v2) { U.push({v1, v2}); }
  //-----------------------------------------------------------------------
  const vec<range> &getU() const { return U; }
  //-----------------------------------------------------------------------
  bool satisfy(vec<int32_t> &pathV, vec<int32_t> &pathE, vec<int64_t> &pathW,
               int32_t cycleIndex) override {
    if (playerSAT == EVEN) {
      for (int32_t i = 0; i < pathV.size(); i++) {
        bool is_safe = false;
        for (int32_t j = 0; j < U.size(); j++) {
          if (pathV[i] >= U[j].first && pathV[i] <= U[j].last) {
            is_safe = true;
            break;
          }
        }
        if (!is_safe) {
          return false;
        }
      }
      return true;
    } else {
      for (int32_t i = 0; i < pathV.size(); i++) {
        bool is_safe = false;
        for (int32_t j = 0; j < U.size(); j++) {
          if (pathV[i] >= U[j].first && pathV[i] <= U[j].last) {
            is_safe = true;
            break;
          }
        }
        if (!is_safe) {
          return true;
        }
      }
      return false;
    }
  }
  //-----------------------------------------------------------------------
  bool satisfySCC(vec<int32_t> pathV, vec<int32_t> &S) override {
    if (playerSAT == EVEN) {
      for (int32_t i = 0; i < pathV.size(); i++) {
        for (int32_t j = 0; j < U.size(); j++) {
          if (pathV[i] >= U[j].first && pathV[i] <= U[j].last) {
            S.push(pathV[i]);
            break;
          }
        }
      }
      return S.size() > 0;
    } else {
      for (int32_t i = 0; i < pathV.size(); i++) {
        bool in_U = false;
        for (int32_t j = 0; j < U.size(); j++) {
          if (pathV[i] >= U[j].first && pathV[i] <= U[j].last) {
            in_U = true;
            break;
          }
        }
        if (!in_U) {
          S.push(pathV[i]);
        }
      }
      return S.size() > 0;
    }
  }
};

//===========================================================================

class BuchiCondition : public WinningCondition {
  using WinningCondition::WinningCondition;

private:
  vec<range> B;

public:
  void pushVertexInB(int32_t v1, int32_t v2) { B.push({v1, v2}); }
  //-----------------------------------------------------------------------
  const vec<range> &getB() const { return B; }
  //-----------------------------------------------------------------------
  bool satisfy(vec<int32_t> &pathV, vec<int32_t> &pathE, vec<int64_t> &pathW,
               int32_t cycleIndex) override {
    if (playerSAT == EVEN) {
      for (int32_t i = cycleIndex + 1; i < pathV.size(); i++) {
        for (int32_t j = 0; j < B.size(); j++) {
          if (pathV[i] >= B[j].first && pathV[i] <= B[j].last) {
            return true;
          }
        }
      }
      return false;
    } else {
      for (int32_t i = cycleIndex + 1; i < pathV.size(); i++) {
        for (int32_t j = 0; j < B.size(); j++) {
          if (pathV[i] >= B[j].first && pathV[i] <= B[j].last) {
            return false;
          }
        }
      }
      return true;
    }
  }
  //-----------------------------------------------------------------------
  bool satisfySCC(vec<int32_t> pathV, vec<int32_t> &S) override {
    if (playerSAT == EVEN) {
      for (int32_t i = 0; i < pathV.size(); i++) {
        for (int32_t j = 0; j < B.size(); j++) {
          if (pathV[i] >= B[j].first && pathV[i] <= B[j].last) {
            S.push(pathV[i]);
            break;
          }
        }
      }
      return S.size() > 0;
    } else {
      for (int32_t i = 0; i < pathV.size(); i++) {
        bool in_B = false;
        for (int32_t j = 0; j < B.size(); j++) {
          if (pathV[i] >= B[j].first && pathV[i] <= B[j].last) {
            in_B = true;
            break;
          }
        }
        if (!in_B)
          S.push(pathV[i]);
      }
      return S.size() > 0;
    }
  }
};

//===========================================================================

class ParityCondition : public WinningCondition {
  using WinningCondition::WinningCondition;

public:
  bool satisfy(vec<int32_t> &pathV, vec<int32_t> &pathE, vec<int64_t> &pathW,
               int32_t cycleIndex) override {
    int64_t m = g.priors[pathV[cycleIndex]];
    for (int32_t i = cycleIndex + 1; i < pathV.size(); i++) {
      if (g.isBetter(g.priors[pathV[i]], m)) {
        m = g.priors[pathV[i]];
      }
    }
    return m % 2 == playerSAT;
  }
  //-----------------------------------------------------------------------
  bool satisfySCC(vec<int32_t> pathV, vec<int32_t> &S) {
    int64_t m = g.priors[pathV[0]];
    S.push(pathV[0]);
    for (int32_t i = 1; i < pathV.size(); i++) {
      if (g.isBetter(g.priors[pathV[i]], m)) {
        m = g.priors[pathV[i]];
        if (m % 2 == playerSAT) {
          S.clear();
          S.push(pathV[i]);
        }
      } else if (g.priors[pathV[i]] == m) {
        if (m % 2 == playerSAT) {
          S.push(pathV[i]);
        }
      }
    }
    if (m % 2 != playerSAT) {
      S.clear();
      return false;
    }
    return true;
  }
};

//===========================================================================

class EnergyCondition : public WinningCondition {
  using WinningCondition::WinningCondition;

private:
  int64_t threshold;

public:
  void setThreshold(int64_t t) { threshold = t; }
  //-----------------------------------------------------------------------
  bool satisfy(vec<int32_t> &pathV, vec<int32_t> &pathE, vec<int64_t> &pathW,
               int32_t cycleIndex) override {
    int64_t sum = 0;
    sum = pathW.last() - pathW[cycleIndex] + g.weights[pathE[cycleIndex]];

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
  //-----------------------------------------------------------------------
  bool satisfy(vec<int32_t> &pathV, vec<int32_t> &pathE, vec<int64_t> &pathW,
               int32_t cycleIndex) override {
    int64_t sum = 0;
    sum = pathW.last() - pathW[cycleIndex] + g.weights[pathE[cycleIndex]];
    double avg = static_cast<double>(sum) / (pathE.size() - cycleIndex);

    if (playerSAT == EVEN) {
      return avg >= threshold;
    }
    return avg < threshold;
  }
};

//===========================================================================
// Other Winning Conditions
//===========================================================================

struct RabinPair {
  vec<int32_t> B, F;
};
//---------------------------------------------------------------------------
class RabinCondition : public WinningCondition {
  using WinningCondition::WinningCondition;

private:
  vec<RabinPair> C;

public:
  void pushRabinPair() { C.push(); }
  //-----------------------------------------------------------------------
  void updateRabinPairB(size_t i, int32_t v) { C[i].B.push(v); }
  //-----------------------------------------------------------------------
  void updateRabinPairF(size_t i, int32_t v) { C[i].F.push(v); }
  //-----------------------------------------------------------------------
  bool satisfy(vec<int32_t> &pathV, vec<int32_t> &pathE, vec<int64_t> &pathW,
               int32_t cycleIndex) override {
    for (size_t p = 0; p < C.size(); p++) {
      bool isB = false;
      bool isF = false;

      for (int32_t i = cycleIndex + 1; i < pathV.size(); i++) {
        int32_t currentState = pathV[i];

        if (!isB) {
          for (size_t j = 0; j < C[p].B.size(); j++) {
            if (currentState == C[p].B[j]) {
              isB = true;
              break;
            }
          }
        }

        if (!isF) {
          for (size_t j = 0; j < C[p].F.size(); j++) {
            if (currentState == C[p].F[j]) {
              isF = true;
              break;
            }
          }
        }

        if (isB)
          break;
      }

      if (!isB && isF)
        return true;
    }

    return false;
  }
};

#endif // WINNING_CONDITIONS_H