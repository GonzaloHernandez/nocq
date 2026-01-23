#ifndef TARJAN_H
#define TARJAN_H

#ifndef GAME_H
#include "game.h"
#endif

#include <vector>

class TarjanSCC {
private:
    Game& g;
    GameView& view;
    std::vector<int>    indices;
    std::vector<int>    lowlink;
    std::vector<bool>   onstack;
    std::vector<int>     stack;
    std::vector<std::vector<int>>   sccs;
    int index = 0;
public:
    TarjanSCC(Game& g,GameView& view);
    
    std::vector<std::vector<int>> solveRAW();
    void searchRAW(int v);

    std::vector<std::vector<int>> solve();
    void search(int v);

    //-------------------------------------------------------------------------
};

#endif // TARJAN_H