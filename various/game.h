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

enum parity_type    {EVEN,ODD};                             // 0,1
enum reward_type    {MIN,MAX};                              // 0,1
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
    std::vector<int>        owners;
    std::vector<long long>  priors;
    std::vector<int>        sources;
    std::vector<int>        targets;
    std::vector<int>        weights;

    std::vector<std::vector<int>>   outs;
    std::vector<std::vector<int>>   ins;
    int nvertices;
    int nedges;
    int init;
    reward_type reward;

    //-------------------------------------------------------------------------

    void fixZeros();
    void parseline_dzn  (const std::string& line,std::vector<int>& myvec);
    void parseline_dzn  (const std::string& line,std::vector<long long>& myvec);
    void parseline_gm   (const std::string& line,std::vector<long long>& vinfo, 
                        std::vector<int>& outs, std::vector<long long>& weights,
                        std::string& comment);

    //-------------------------------------------------------------------------

public:
    
    Game(   std::vector<int> owners,std::vector<long long> priors,
            std::vector<int> sources,std::vector<int> targets,
            std::vector<int> wei={},int init=0, reward_type rew=MAX);

    Game(   int type, std::string filename, std::vector<int> weights, 
            int init=0, reward_type rew=MAX);

    Game(   int type, std::vector<int> vals, std::vector<int> weights, 
            int init=0, reward_type rew=MAX);

    void setInit(int init);
    void setReward(reward_type rew);
    bool comparePriorities(int p1,int p2,parity_comp rel=BET);
    void exportFile(int type, std::string filename);
    void printGame();
    void flipGame();
};

// ============================================================================

class GameView {
private:
    Game& g;
public:
    std::unique_ptr<bool[]> vs;
    std::unique_ptr<bool[]> es;
    GameView(Game& g);

    //-------------------------------------------------------------------------

    std::vector<int> getVertices();     // Only return a set of active vertices
    std::vector<int> getEdges();        // Only return a set of active edges
    std::vector<int> getOuts(int v);    // Only return a set of active outs of v
    std::vector<int> getIns(int w);     // Only return a set of active ins of w
    std::string viewCurrent();

    void activeAll();
    void deactiveAll();
};

#endif // GAME_H