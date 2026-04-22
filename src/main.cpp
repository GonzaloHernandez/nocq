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

//-----------------------------------------------------------------------------

struct options {
    bool printGame          = false; 
    bool printSolution      = false; 
    bool printStatistics    = false; 
    bool printVerbose       = false; 
    int  printTime          = 0;        // 0=Default 1=Solving Time 2=All-times
    game_type  gameType     = DEF;      // DEF,JURD,RAND,MLADDER,DZN,GM,GMW,DIM

    objective_type  objective       = MAX;          // MAXimize,MINimize
    vec<int32_t>    vals;
    float           lbound          = 0.0;
    float           ubound          = 0.0;
    vec<int32_t>    init;
    std::string     gameFilename    = "";
    std::string     exportFilename  = "";
    int             exportType      = 0;            // 0=not DZN,GM,GMW,DIM
    std::string     method          = "";           // NOC-EVEN,NOC-ODD,SAT
                                                    // ZRA,FRA,SCC
    std::string     solver          = "chuffed";    // chuffed, gecode,
                                                    // chuffed-int
    bool            flip            = false;
    bool            parityCond      = false;
    bool            energyCond      = false;
    bool            meanpayoffCond  = false;
    float           threshold       = 0.0;
} options;

//-----------------------------------------------------------------------------

bool parseMyOptions(int argc, char *argv[]) {
    options.init.push(0);
    int i=1;
    //-------------------------------------------------------------------------
    auto validateArg = [&](const char* flagName) -> char* {
        i++; // Move to the next argument
        if (i>=argc || (strlen(argv[i])>1 && strncmp(argv[i],"--",2) == 0)) {
            std::cerr << "ERROR: Value for [" << flagName << "] is missing\n";
            exit(0);
        }
        return argv[i];
    };
    //-------------------------------------------------------------------------
    auto parseInteger = [&](const char* str, int min, int max) -> int {
        char* endptr;
        errno = 0;
        long val = std::strtol(str, &endptr, 10);

        if (errno == ERANGE || val < min || val > max) {
            std::cerr   << "ERROR: Value [" << str << "] out of range (" 
                        << min << "-" << max << ")\n";
            exit(1);
        }
        if (*endptr != '\0') {
            std::cerr<< "ERROR: Value [" << str << "] is not a valid number\n";
            exit(1);
        }
        return (int)val;
    };
    //-------------------------------------------------------------------------
    auto parseFloat = [&](const char* str, float min, float max) -> float {
        char* endptr;
        errno = 0;
        float val = std::strtof(str, &endptr);
        if (errno == ERANGE) {
            std::cerr << "ERROR: Value [" << str << "] out of floating-point range\n";
            exit(1);
        }
        
        if (val < min || val > max) {
            std::cerr << "ERROR: Value [" << str << "] out of bounds (" 
                    << min << "-" << max << ")\n";
            exit(1);
        }
        if (*endptr != '\0') {
            const char* check = endptr;
            while (std::isspace(*check)) check++;
            if (*check != '\0') {
                std::cerr << "ERROR: Value [" << str << "] contains invalid characters\n";
                exit(1);
            }
        }        
        return val;
    };
    //-------------------------------------------------------------------------
    so.nof_solutions = 1;
    for (; i<argc; i++) {
        if (strcmp(argv[i],"--jurd")==0) {
            validateArg("--jurd <levels>");
            options.vals.push(parseInteger(argv[i], 2, 1000000));
            validateArg("--jurd <blocks>");
            options.vals.push(parseInteger(argv[i], 1, 1000000));
            options.gameType = JURD;
        }
        else if (strcmp(argv[i],"--rand")==0) {
            validateArg("--rand <vertices>");
            options.vals.push(parseInteger(argv[i], 1, 10000000));
            validateArg("--rand <priorities>");
            options.vals.push(parseInteger(argv[i], 1, 10000000));
            validateArg("--rand <min edges>");
            options.vals.push(parseInteger(argv[i], 1, 199));
            validateArg("--rand <max edges>");
            options.vals.push(parseInteger(argv[i], 2, 300));
            options.gameType = RAND;
        }
        else if (strcmp(argv[i],"--mladder")==0) {
            validateArg("--mladder <blocks>");
            options.vals.push(parseInteger(argv[i], 1, 1000000));
            options.gameType = MLADDER;
        }
        else if (strcmp(argv[i],"--weights")==0) {
            validateArg("--weights <lower_bound upper_bound>");
            options.lbound = parseFloat(argv[i], -1000000.0, 1000000.0);
            validateArg("--weights <final weight>");
            options.ubound = parseFloat(argv[i], -1000000.0, 1000000.0);
        }
        else if (strcmp(argv[i],"--dzn")==0) {
            options.gameType = DZN;
            validateArg("--dzn <filename>");
            options.gameFilename = argv[i];                
        }
        else if (strcmp(argv[i],"--gm")==0) {
            options.gameType = GM;
            validateArg("--gm <filename>");
            options.gameFilename = argv[i];                
        }        
        else if (strcmp(argv[i],"--init")==0) {
            validateArg("--init <initial_vertex>");

            options.init.clear();
            std::string s = argv[i];
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, ',')) {
                size_t init = item.find_first_not_of(" \t");
                size_t end = item.find_last_not_of(" \t");
                if (init == std::string::npos || end == std::string::npos) {
                    std::cerr << "ERROR: Invalid values for [--init]\n";
                    return false;
                }
                options.init.push(std::stoi(item));
            }
        }
        else if (strcmp(argv[i],"--export-dzn")==0) {
            validateArg("--export-dzn <filename>");
            options.exportType = DZN;
            options.exportFilename = argv[i];                
        }
        else if (strcmp(argv[i],"--export-gm")==0) {
            validateArg("--export-gm <filename>");
            options.exportType = GM;
            options.exportFilename = argv[i];                
        }
        else if (strcmp(argv[i],"--export-gmw")==0) {
            validateArg("--export-gmw <filename>");
            options.exportType = GMW;
            options.exportFilename = argv[i];                
        }
        else if (strcmp(argv[i],"--sat-encoding")==0) {
            validateArg("--sat-encoding <filename>");
            options.method = "SAT";
            options.exportType = DIM;
            options.exportFilename = argv[i];
        }
        else if (strcmp(argv[i],"--nsolutions")==0) {
            validateArg("--nsolutions <number>");
            so.nof_solutions = parseInteger(argv[i], 0, 10);
        }
        else if (strcmp(argv[i],"--threshold")==0) {
            validateArg("--threshold <value>");
            options.threshold = parseFloat(argv[i], -1000000.0, 1000000.0);
        }

        else if (strcmp(argv[i],"--max")==0)
                                { options.objective         = MAX; }
        else if (strcmp(argv[i],"--min")==0)
                                { options.objective         = MIN; }
        else if (strcmp(argv[i],"--testing")==0)
                                { options.method            = "testing"; }
        else if (strcmp(argv[i],"--noc")==0)
                                { options.method            = "noc-even"; }
        else if (strcmp(argv[i],"--noc-even")==0)
                                { options.method            = "noc-even"; }
        else if (strcmp(argv[i],"--noc-odd")==0)
                                { options.method            = "noc-odd"; }
        else if (strcmp(argv[i],"--zra")==0)
                                { options.method            = "zra"; }
        else if (strcmp(argv[i],"--fra")==0)
                                { options.method            = "fra"; }
        else if (strcmp(argv[i],"--scc")==0)
                                { options.method            = "scc"; }
        else if (strcmp(argv[i],"--chuffed")==0)
                                { options.solver          = "chuffed"; }
        else if (strcmp(argv[i],"--gecode")==0)
                                { options.solver          = "gecode"; }
        else if (strcmp(argv[i],"--chuffed-int")==0)
                                { options.solver          = "chuffed-int"; }
        else if (strcmp(argv[i],"--print-only-times")==0)
                                { options.printTime        = -2; }
        else if (strcmp(argv[i],"--print-only-time")==0)
                                { options.printTime        = -1; }
        else if (strcmp(argv[i],"--print-time")==0)
                                { options.printTime        = 1; }
        else if (strcmp(argv[i],"--print-times")==0)
                                { options.printTime        = 2; }
        else if (strcmp(argv[i],"--print-game")==0)
                                { options.printGame        = true; }
        else if (strcmp(argv[i],"--print-solution")==0)
                                { options.printSolution    = true; }
        else if (strcmp(argv[i],"--print-statistics")==0)
                                { options.printStatistics  = true; }
        else if (strcmp(argv[i],"--verbose")==0)
                                { options.printVerbose     = true; }
        else if (strcmp(argv[i],"--flip")==0)
                                { options.flip              = true;}
        else if (strcmp(argv[i],"--parity")==0)
                                { options.parityCond       = true; }
        else if (strcmp(argv[i],"--energy")==0)
                                { options.energyCond       = true; }
        else if (strcmp(argv[i],"--mean-payoff")==0)
                                { options.meanpayoffCond   = true; }

        else if (strcmp(argv[i],"--help")==0) {
            std::cout << "Usage: " << argv[0] << " [options] <args>\n"
            << "Options:\n"
            << "  --dzn <filename>           : DZN file name\n"
            << "  --gm <filename>            : GM file name\n"
            << "  --jurd <levels> <blocks>   : Jurdzinski game\n"
            << "  --rand <ns> <ps> <d1> <d2> : Random game\n"
            << "  --mladder <bl>             : ModelcheckerLadder game\n"
            << "  --weights <w1> <w2>        : Weights range\n"
            << "  --init <vertex>            : Initial vertex\n"
            << "  --print-only-time          : Print only solving time\n"
            << "  --print-only-times         : Print preptime+solving time\n"
            << "  --print-time               : Print solving time\n"
            << "  --print-times              : Print all times\n"
            << "  --print-game               : Print game\n"
            << "  --print-solution           : Print solution (All vertices)\n"
            << "  --print-statistics         : Print statistics after solving\n"
            << "  --verbose                  : Print everything\n"
            << "  --max                      : Seek to maximize the priority\n"
            << "  --min                      : Seek to minimize the priority\n"
            << "  --export-dzn <filename>    : Export game to DZN format\n"
            << "  --export-gm <filename>     : Export game to GM format\n"
            << "  --export-gmw <filename>    : Export game to GM + Weights\n"
            << "  --noc-even                 : CP-NOC satisfying player EVEN\n"
            << "  --noc-odd                  : CP-NOC satisfying player ODD\n"
            << "  --chuffed                  : CP Solver (Chuffed using BoolVars)\n"
            << "  --chuffed-int              : CP Solver (Chuffed using IntVars)\n"
            << "  --gecode                   : CP Solver (Gecode)\n"
            << "  --sat-encoding <filename>  : Encode on DIMACS file\n"
            << "  --fra                      : Solve using FRA\n"
            << "  --flip                     : Complement the game\n"
            << "  --threshold <value>        : Threshold for MeanPayoff\n"
            << "  --parity                   : Parity condition (default)\n"
            << "  --energy                   : Energy condition\n"
            << "  --mean-payoff              : Mean-Payoff condition\n";
            exit(0);
        }
        else {
            std::cerr << "ERROR: Unknown option: " << argv[i] << std::endl;
            exit(0);
        }
    }
    return true;
}

//=============================================================================

int main(int argc, char *argv[])
{
    launchdbg();
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
        case JURD: case RAND: case MLADDER:
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

    if (options.exportType == DZN) {
        game->exportFile(DZN, options.exportFilename);
    }
    else if (options.exportType == GM) {
        game->exportFile(GM, options.exportFilename);
    }
    else if (options.exportType == GMW) {
        game->exportFile(GMW, options.exportFilename);
    }

    // Ensure at least one winning condition is selected, default --parity
    if (!options.parityCond && !options.energyCond && 
        !options.meanpayoffCond) 
    {
        options.parityCond = true;
    }

    //-------------------------------------------------------------------------
    // For testing purposes

    if (options.method=="testing") {
    }

    //-------------------------------------------------------------------------
    // CP-NOC-Chuffed

    else if (options.method.substr(0,3)=="noc"&&options.solver=="chuffed") {
        vec<bool> win_conditions;
        win_conditions.push(options.parityCond);
        win_conditions.push(options.energyCond);
        win_conditions.push(options.meanpayoffCond);
        startClock(); //.............................................
        Chuffed::NOCModel* model = new Chuffed::NOCModel(
                            *game, win_conditions, options.threshold, 
                            (options.printSolution || options.printVerbose),
                            options.method=="noc-even"?EVEN:ODD);

        so.print_sol = options.printSolution || options.printVerbose;
        double preptime = stopClock(); //............................

        if (options.printTime>1) {
            std::cout << "Init time          : " << preptime << std::endl;
        }

        startClock(); //.............................................
        if (options.printSolution || options.printVerbose) {
            engine.solve(model);
        }
        else {
            std::streambuf* old_buf = std::cout.rdbuf();
            std::ofstream null_stream("/dev/null");
            std::cout.rdbuf(null_stream.rdbuf());
            engine.solve(model);
            std::cout.rdbuf(old_buf);
        }
        double totaltime = stopClock(); //...........................

        if (options.printTime>1 || options.printVerbose) {
            std::cout << "Solving time       : " << totaltime << std::endl;
            std::cout << "Mem used           : " << memUsed() << std::endl;
        }

        if (options.method=="noc-even") {
            if (options.printTime>=0 || options.printVerbose)
                std::cout   << game->init << ": " 
                            << (engine.solutions>0?"EVEN ":"ODD ");
        }
        else {
            if (options.printTime>=0 || options.printVerbose)
                std::cout   << game->init << ": " 
                            << (engine.solutions>0?"ODD ":"EVEN ");
        }

        if (options.printTime<=-2 || options.printVerbose) {
            std::cout   << preptime << "\t";
        }

        if (options.printTime!=0 || options.printVerbose) {
            std::cout   << totaltime;
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
        vec<bool> win_conditions;
        win_conditions.push(options.parityCond);
        win_conditions.push(options.energyCond);
        win_conditions.push(options.meanpayoffCond);
        startClock(); //.............................................
        ChuffedInt::NOCModel* model = new ChuffedInt::NOCModel(
                            *game, win_conditions, options.threshold, 
                            (options.printSolution || options.printVerbose),
                            options.method=="noc-even"?EVEN:ODD);

        so.print_sol = options.printSolution || options.printVerbose;
        double preptime = stopClock(); //............................

        if (options.printTime>1) {
            std::cout << "Init time          : " << preptime << std::endl;
        }

        startClock(); //.............................................
        if (options.printSolution || options.printVerbose) {
            engine.solve(model);
        }
        else {
            std::streambuf* old_buf = std::cout.rdbuf();
            std::ofstream null_stream("/dev/null");
            std::cout.rdbuf(null_stream.rdbuf());
            engine.solve(model);
            std::cout.rdbuf(old_buf);
        }
        double totaltime = stopClock(); //...........................

        if (options.printTime>1 || options.printVerbose) {
            std::cout << "Solving time       : " << totaltime << std::endl;
            std::cout << "Mem used           : " << memUsed() << std::endl;
        }

        if (options.method=="noc-even") {
            if (options.printTime>=0 || options.printVerbose)
                std::cout   << game->init << ": " 
                            << (engine.solutions>0?"EVEN ":"ODD ");
        }
        else {
            if (options.printTime>=0 || options.printVerbose)
                std::cout   << game->init << ": " 
                            << (engine.solutions>0?"ODD ":"EVEN ");
        }

        if (options.printTime<=-2 || options.printVerbose) {
            std::cout   << preptime << "\t";
        }

        if (options.printTime!=0 || options.printVerbose) {
            std::cout   << totaltime;
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
        vec<bool> win_conditions;
        win_conditions.push(options.parityCond);
        win_conditions.push(options.energyCond);
        win_conditions.push(options.meanpayoffCond);

        startClock(); //.............................................
        Gecode::NocModel* model = new Gecode::NocModel(
                            *game, win_conditions, options.threshold, 
                            options.method=="noc-even"?EVEN:ODD);

        double preptime = stopClock(); //............................

        if (options.printTime>1) {
            std::cout << "Init time          : " << preptime << std::endl;
        }

        startClock(); //.............................................
        Gecode::DFS<Gecode::NocModel> dfs(model);
        delete model;
        Gecode::NocModel* solution = dfs.next();

        if (options.printSolution || options.printVerbose) {
            if (solution) solution->print();
            else std::cout << "UNSATISFIABLE" << std::endl;
        }
        double totaltime = stopClock(); //...........................

        if (options.printTime>1 || options.printVerbose) {
            std::cout << "Solving time       : " << totaltime << std::endl;
            std::cout << "Mem used           : " << "???" << std::endl;
        }

        if (options.method=="noc-even") {
            if (options.printTime>=0 || options.printVerbose)
                std::cout << game->init << ": " << (solution?"EVEN ":"ODD ");
        }
        else {
            if (options.printTime>=0 || options.printVerbose)
                std::cout << game->init << ": " << (!solution?"ODD ":"EVEN ");
        }

        if (options.printTime<=-2 || options.printVerbose) {
            std::cout   << preptime << "\t";
        }

        if (options.printTime!=0 || options.printVerbose) {
            std::cout   << totaltime;
        }        

        std::cout << std::endl;

        if (options.printStatistics || options.printVerbose) {
            // engine.printStats();
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

        if (options.printVerbose) {
            std::cout << "Preparation time   : " << preptime << std::endl;
        }

        startClock(); //.............................................
        auto win = zlk.solve();
        double totaltime = stopClock(); //...........................

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

        for (size_t i=0; i<options.init.size(); i++) {
            int32_t v0 = options.init[i];
            auto it = std::find(win[0].begin(), win[0].end(), v0);

            if (options.printTime>=0 || options.printVerbose)
                std::cout << v0 << ": " << (it != win[0].end()?"EVEN ":"ODD ");
            
            
            if (options.printTime!=0 || options.printVerbose) {
                std::cout   << totaltime;
            }

            std::cout << std::endl;        
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