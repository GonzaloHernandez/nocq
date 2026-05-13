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
#include "utils/dbg.h"
#include "utils/parameters.h"
#include "utils/fra.h"
#include "utils/game.h"
#include "utils/tarjan.h"
#include "utils/zielonka.h"
#include "utils/satencoder.h"
#include "cp_nocq/nocq_chuffed.cpp"
#include "cp_nocq/nocq_chuffed_int.cpp"

#ifdef HAS_GECODE
#include "cp_nocq/nocq_gecode.cpp"
#endif

//=============================================================================

int main(int argc, char *argv[])
{
    launchdbg();
    so.nof_solutions = 1;
    parseMyOptions(argc, argv);
    Game* game = nullptr;

    //-------------------------------------------------------------------------

    std::chrono::high_resolution_clock::time_point clockStorage;

    auto startClock = [&]() {
        clockStorage = std::chrono::high_resolution_clock::now();
    };

    auto stopClock = [&]() -> double {
        auto endTime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(endTime - clockStorage).count();
    };

    //-------------------------------------------------------------------------

    startClock(); //.............................................
    switch (options.gameType) {
        case DZN: case GM:
            game = new Game(options.gameType, 
                            options.gameFilename, 
                            options.init[0],
                            options.objective,
                            options.lbound, options.ubound);
            break;
        case JURD: case RAND: case MLADDER: case SPRAND: case SQNC:
            game = new Game(options.gameType, 
                            options.vals,
                            options.init[0],
                            options.objective,
                            options.lbound, options.ubound);
            break;
        default:
            return 0;
    }
    double launchinggame = stopClock(); //...........................

    if (options.flip) game->flipGame();

    if (options.printGame || options.printVerbose) {
        game->printGame();
    }

    if ((options.printTime>1 || options.printVerbose)) {
        std::cout << "Game creation time : " << launchinggame << std::endl;
    }
    else if (options.printTime<-1) {
        std::cout << launchinggame << " " << std::flush;
    }

    if (options.exportType != DEF) {
        game->exportFile(options.exportType, options.exportFilename);
    }

    // Ensure at least one winning condition is selected, default --parity
    if (!options.parityCond && !options.energyCond && 
        !options.meanpayoffCond) 
    {
        options.parityCond = true;
    }

    vec<WinningCondition*> winConditions;
    if (options.parityCond) {
        ParityCondition* c = new ParityCondition(*game,
                                    options.method=="noc-even"?EVEN:ODD);
        winConditions.push(c);
    }
    if (options.energyCond) {
        EnergyCondition* c = new EnergyCondition(*game,
                                    options.method=="noc-even"?EVEN:ODD);
        c->setThreshold(options.thresholdEnergy);
        winConditions.push(c);
    }
    if (options.meanpayoffCond) {
        MeanPayoffCondition* c = new MeanPayoffCondition(*game,
                                    options.method=="noc-even"?EVEN:ODD);
        c->setThreshold(options.thresholdMPG);
        winConditions.push(c);
    }

    //-------------------------------------------------------------------------
    // For testing purposes

    if (options.method=="testing") {
    }

    //-------------------------------------------------------------------------
    // CP-NOC-Chuffed

    else if (   options.method.substr(0,3)=="noc" &&
                options.solver=="chuffed") 
    {
        startClock(); //.............................................
        Chuffed::NOCModel* model = new Chuffed::NOCModel(
                            *game, winConditions, 
                            (options.printSolution || options.printVerbose),
                            options.method=="noc-even"?EVEN:ODD);

        so.print_sol = options.printSolution || options.printVerbose;
        double preptime = stopClock(); //............................

        if (options.printTime>1 || options.printVerbose) {
            std::cout << "Init time          : " << preptime << std::endl;
        }
        else if (options.printTime<-1) {
            std::cout   << preptime << " " << std::flush;
        }

        std::streambuf* old_buf = std::cout.rdbuf();
        std::stringstream ss;
        std::cout.rdbuf(ss.rdbuf());

        startClock(); //.............................................
        engine.solve(model);
        double totaltime = stopClock(); //...........................

        std::cout.rdbuf(old_buf);

        std::string answer = "";
        if ((options.method == "noc-even" && engine.solutions) ||
            (options.method != "noc-even" && !engine.solutions) ) {
            answer = "EVEN";
        } else {
            answer = "ODD";
        }

        if (options.printTime>1 || options.printVerbose) {
            std::cout << "Solving time       : " << totaltime << std::endl;
            // std::cout << "Mem used           : " << memUsed() << std::endl;
        }
        else if (options.printTime<0) {
            std::cout   << totaltime << " " << std::flush;
        }
        
        if (options.printTime == 1) {
            std::cout   << totaltime << " " << std::flush;
        }

        if (options.printTime>=0 || options.printVerbose) {
            std::cout   << answer;
        }

        if (options.printSolution || options.printVerbose) {
            std::string solver_output = ss.str();
            std::cout << "\n" << solver_output;
        }

        std::cout << std::endl;

        if (options.printStatistics || options.printVerbose) {
            engine.printStats();
        }
        
        delete model;
    }

    //-------------------------------------------------------------------------
    // CP-NOC-Chuffed-Int

    else if (   options.method.substr(0,3)=="noc" &&
                options.solver=="chuffed-int") 
    {
        startClock(); //.............................................
        ChuffedInt::NOCModel* model = new ChuffedInt::NOCModel(
                            *game, winConditions,
                            (options.printSolution || options.printVerbose),
                            options.method=="noc-even"?EVEN:ODD);

        so.print_sol = options.printSolution || options.printVerbose;
        double preptime = stopClock(); //............................

        if (options.printTime>1 || options.printVerbose) {
            std::cout << "Init time          : " << preptime << std::endl;
        }
        else if (options.printTime<-1) {
            std::cout   << preptime << " " << std::flush;
        }

        std::streambuf* old_buf = std::cout.rdbuf();
        std::stringstream ss;
        std::cout.rdbuf(ss.rdbuf());

        startClock(); //.............................................
        engine.solve(model);
        double totaltime = stopClock(); //...........................

        std::cout.rdbuf(old_buf);

        std::string answer = "";
        if ((options.method == "noc-even" && engine.solutions) ||
            (options.method != "noc-even" && !engine.solutions) ) {
            answer = "EVEN";
        } else {
            answer = "ODD";
        }

        if (options.printTime>1 || options.printVerbose) {
            std::cout << "Solving time       : " << totaltime << std::endl;
            // std::cout << "Mem used           : " << memUsed() << std::endl;
        }
        else if (options.printTime<0) {
            std::cout   << totaltime << " " << std::flush;
        }
        
        if (options.printTime == 1) {
            std::cout   << totaltime << " " << std::flush;
        }

        if (options.printTime>=0 || options.printVerbose) {
            std::cout   << answer;
        }

        if (options.printSolution || options.printVerbose) {
            std::string solver_output = ss.str();
            std::cout << "\n" << solver_output;
        }

        std::cout << std::endl;

        if (options.printStatistics || options.printVerbose) {
            engine.printStats();
        }
        
        delete model;
    }
    
    //-------------------------------------------------------------------------
    // CP-NOC-Gecode

    else if (options.method.substr(0,3)=="noc"&&options.solver=="gecode") {

    #ifdef HAS_GECODE
        startClock(); //.............................................
        Gecode::NocModel* model = new Gecode::NocModel(
                            *game, winConditions,
                            options.method=="noc-even"?EVEN:ODD);

        double preptime = stopClock(); //............................

        if (options.printTime>1 || options.printVerbose) {
            std::cout << "Init time          : " << preptime << std::endl;
        }
        else if (options.printTime<-1) {
            std::cout   << preptime << " " << std::flush;
        }

        startClock(); //.............................................
        Gecode::DFS<Gecode::NocModel> dfs(model);
        delete model;
        Gecode::NocModel* solution = dfs.next();
        double totaltime = stopClock(); //...........................

        std::string answer = "";
        if ((options.method == "noc-even" && solution) ||
            (options.method != "noc-even" && !solution) ) {
            answer = "EVEN";
        } else {
            answer = "ODD";
        }

        if (options.printTime>1 || options.printVerbose) {
            std::cout << "Solving time       : " << totaltime << std::endl;
            // std::cout << "Mem used           : " << "???" << std::endl;
        } if (options.printTime<0) {
            std::cout   << totaltime << " " << std::flush;
        }

        if (options.printTime == 1) {
            std::cout   << totaltime << " " << std::flush;
        }

        if (options.printTime>=0 || options.printVerbose) {
            std::cout   << answer;
        }

        if (options.printSolution || options.printVerbose) {
            std::cout << std::endl;
            if (solution) solution->print();
            else std::cout << "UNSATISFIABLE" << std::endl;
        }

        std::cout << std::endl;

        if (options.printStatistics || options.printVerbose) {
            std::cout << "Statistics";
        }
        
        if (solution) delete solution;

    #else
        std::cout << "Error: Gecode support is disabled.\n" 
                  << "Please rebuild NOCQ using -DENABLE_GECODE=ON\n";
                  

    #endif //HAS_GECODE

    }

    //-------------------------------------------------------------------------
    // SAT Encoding

    else if (options.method == "sat") {
        SATEncoder encoder(*game);

        startClock(); //.............................................
        auto cnf = encoder.getCNF();
        double encodetime = stopClock(); //..........................

        startClock(); //.............................................
        encoder.dimacs(cnf,options.exportFilename);
        double dimacstime = stopClock(); //..........................

        if (options.printTime>=0 || options.printVerbose) {
            std::cout << "Encoding time      : " << encodetime << std::endl;
            std::cout << "Dimacs time        : " << dimacstime << std::endl;
        } 
    }

    //-------------------------------------------------------------------------
    // ZRA

    else if (options.method=="zra") {

        startClock(); //.............................................
        Zielonka zlk(*game);
        double preptime = stopClock(); //............................

        if (options.printTime>1 || options.printVerbose) {
            std::cout << "Init time          : " << preptime << std::endl;
        }
        else if (options.printTime<-1) {
            std::cout   << preptime << " " << std::flush;
        }

        startClock(); //.............................................
        auto win = zlk.solve();
        double totaltime = stopClock(); //...........................

        if (options.printTime>1 || options.printVerbose) {
            std::cout << "Solving time       : " << totaltime << std::endl;
            // std::cout << "Mem used           : " << memUsed() << std::endl;
        }
        else if (options.printTime<0) {
            std::cout   << totaltime << " " << std::flush;
        }

        if (options.printTime == 1) {
            std::cout   << totaltime << " " << std::flush;
        }

        for (size_t i=0; i<options.init.size(); i++) {
            int32_t v0 = options.init[i];
            auto it = std::find(win[0].begin(), win[0].end(), v0);

            if (options.printTime>=0 || options.printVerbose)
                std::cout << v0 << ": " << (it != win[0].end()?"EVEN ":"ODD ");
            
            std::cout << std::endl;        
        }

        if (options.printSolution || options.printVerbose) {
            std::cout << "EVEN {";
            for (int i = 0; i < win[0].size(); i++) {
                std::cout << win[0][i];
                if (i<win[0].size()-1) std::cout << ",";
            }
            std::cout << "}\nODD  {";
            for (int i = 0; i < win[1].size(); i++) {
                std::cout << win[1][i];
                if (i<win[1].size()-1) std::cout << ",";
            }
            std::cout << "}" <<std::endl;
        }


    }

    //-------------------------------------------------------------------------
    // FRA

    else if (options.method=="fra") {
        if (options.printSolution || options.printVerbose) {
            options.init.growTo(game->nvertices);
            for (int32_t v=0; v<game->nvertices; v++) options.init[v]=v;
        }
        for (size_t i=0; i<options.init.size(); i++) {
            int32_t v = options.init[i];
            startClock(); //.............................................
            auto play = getPlay(*game, v, true);
            double totaltime = stopClock(); //...........................

            if (options.printTime>=0 || options.printVerbose)
                std::cout << v << ": " << (play==EVEN?"EVEN ":"ODD "); 

            if (options.printTime!=0 || options.printVerbose) {
                std::cout   << totaltime;
            }

            std::cout << std::endl;
        }
    }

    //-------------------------------------------------------------------------
    // SCC

    else if (options.method=="scc") { 
        GameView view(*game);
        TarjanSCC tscc(*game,view);
        auto sccs = tscc.solve();
        int counter = 0;
        for (auto& scc : sccs) {
            std::cout << "{";
            for (int i = 0; i < scc.size(); i++) {
                std::cout << scc[i];
                if (i<scc.size()-1) std::cout << ",";
            }
            std::cout << "}" << std::endl;
            counter += 1;
        }
        std::cout << "Total SCCs: " << counter << std::endl;
    }

    //-------------------------------------------------------------------------

    delete game;

    return 0;
}