#ifndef ZIELONKA_H
#define ZIELONKA_H

#ifndef GAME_H
#include "game.h"
#endif

#include "iostream"

class Zielonka {
private:
    Game& g;
public:
    //-------------------------------------------------------------------------
    Zielonka(Game& g);

    std::vector<int> getBestVertices(bool* removed);
    void attractor(int player, std::vector<int>&U, bool* removed);
    std::array<std::vector<int>,2> search(bool* removed, int level=0);
    std::array<std::vector<int>,2> solve();
};

#endif // ZIELONKA_H