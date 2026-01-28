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
#include "utils/fra.h"
#include "utils/game.h"
#include "utils/tarjan.h"
#include "utils/zielonka.h"
#include "utils/satencoder.h"
#include "cp_nocq/nocq_chuffed.cpp"

#ifdef HAS_GECODE
#include "cp_nocq/nocq_gecode.cpp"
#endif
//-----------------------------------------------------------------------------

struct options {
    bool print_game         = false; 
    bool print_solution     = false; 
    bool print_statistics   = false; 
    bool print_verbose      = false; 
    int  print_time         = 0;        // 0=Default 1=Solving Time 2=All-times
    int  game_type          = DEF;      // DEF,JURD,RAND,MLADDER,DZN,GM,GMW,DIM

    reward_type         reward          = MAX;          // MAXimize,MINimize
    std::vector<int>    vals            = {};
    std::vector<int>    weights         = {0,0,0};      // lBound,uBound,Force
    std::vector<int>    init            = {0};
    std::string         game_filename   = "";
    std::string         export_filename = "";
    int                 export_type     = 0;            // 0=not DZN,GM,GMW,DIM
    std::string         solver          = "";           // NOC-EVEN,NOC-ODD,SAT
                                                        // ZRA,FRA,SCC
    std::string         cpengine        = "chuffed";    // chuffed,gecode
    bool                flip            = false;        // 0=no 1=flip

    std::vector<bool>   win_conditions  = {false,false,false};  // 0=parity 
                                                                // 1=energy 
                                                                // 2=meanPayoff
    int                 threshold       = 0;                    // threshold

} options;

//-----------------------------------------------------------------------------

bool parseMyOptions(int argc, char *argv[]) {
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
    so.nof_solutions = 1;
    for (; i<argc; i++) {
        if (strcmp(argv[i],"--jurd")==0) {
            validateArg("--jurd <levels>");
            options.vals.push_back(parseInteger(argv[i], 2, 1000000));
            validateArg("--jurd <blocks>");
            options.vals.push_back(parseInteger(argv[i], 1, 1000000));
            options.game_type = JURD;
        }
        else if (strcmp(argv[i],"--rand")==0) {
            validateArg("--rand <vertices>");
            options.vals.push_back(parseInteger(argv[i], 1, 10000000));
            validateArg("--rand <priorities>");
            options.vals.push_back(parseInteger(argv[i], 1, 10000000));
            validateArg("--rand <min edges>");
            options.vals.push_back(parseInteger(argv[i], 1, 199));
            validateArg("--rand <max edges>");
            options.vals.push_back(parseInteger(argv[i], 2, 300));
            options.game_type = RAND;
        }
        else if (strcmp(argv[i],"--mladder")==0) {
            validateArg("--mladder <blocks>");
            options.vals.push_back(parseInteger(argv[i], 1, 1000000));
            options.game_type = MLADDER;
        }
        else if (strncmp(argv[i],"--weights",9)==0) {
            if (strcmp(argv[i],"--weights-force")==0) options.weights[2] = 1;
            validateArg("--weights <initial weight>");
            options.weights[0] = parseInteger(argv[i], -1000000, 1000000);
            validateArg("--weights <final weight>");
            options.weights[1] = parseInteger(argv[i], -1000000, 1000000);
        }
        else if (strcmp(argv[i],"--dzn")==0) {
            options.game_type = DZN;
            validateArg("--dzn <filename>");
            options.game_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--gm")==0) {
            options.game_type = GM;
            validateArg("--gm <filename>");
            options.game_filename = argv[i];                
        }        
        else if (strcmp(argv[i],"--init")==0) {
            validateArg("--init <initial vertex>");

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
                options.init.push_back(std::stoi(item));
            }
        }
        else if (strcmp(argv[i],"--export-dzn")==0) {
            validateArg("--export-dzn <filename>");
            options.export_type = DZN;
            options.export_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--export-gm")==0) {
            validateArg("--export-gm <filename>");
            options.export_type = GM;
            options.export_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--export-gmw")==0) {
            validateArg("--export-gmw <filename>");
            options.export_type = GMW;
            options.export_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--sat-encoding")==0) {
            validateArg("--sat-encoding <filename>");
            options.solver = "SAT";
            options.export_type = DIM;
            options.export_filename = argv[i];
        }
        else if (strcmp(argv[i],"--nsolutions")==0) {
            validateArg("--nsolutions <number>");
            so.nof_solutions = parseInteger(argv[i], 0, 10);
        }
        else if (strcmp(argv[i],"--threshold")==0) {
            validateArg("--threshold <value>");
            options.threshold = parseInteger(argv[i], -1000000, 1000000);
        }

        else if (strcmp(argv[i],"--max")==0)
                                { options.reward            = MAX; }
        else if (strcmp(argv[i],"--min")==0)
                                { options.reward            = MIN; }
        else if (strcmp(argv[i],"--testing")==0)
                                { options.solver            = "testing"; }
        else if (strcmp(argv[i],"--noc")==0)
                                { options.solver            = "noc-even"; }
        else if (strcmp(argv[i],"--noc-even")==0)
                                { options.solver            = "noc-even"; }
        else if (strcmp(argv[i],"--noc-odd")==0)
                                { options.solver            = "noc-odd"; }
        else if (strcmp(argv[i],"--zra")==0)
                                { options.solver            = "zra"; }
        else if (strcmp(argv[i],"--fra")==0)
                                { options.solver            = "fra"; }
        else if (strcmp(argv[i],"--scc")==0)
                                { options.solver            = "scc"; }
        else if (strcmp(argv[i],"--chuffed")==0)
                                { options.cpengine          = "chuffed"; }
        else if (strcmp(argv[i],"--gecode")==0)
                                { options.cpengine          = "gecode"; }
        else if (strcmp(argv[i],"--print-only-times")==0)
                                { options.print_time        = -2; }
        else if (strcmp(argv[i],"--print-only-time")==0)
                                { options.print_time        = -1; }
        else if (strcmp(argv[i],"--print-time")==0)
                                { options.print_time        = 1; }
        else if (strcmp(argv[i],"--print-times")==0)
                                { options.print_time        = 2; }
        else if (strcmp(argv[i],"--print-game")==0)
                                { options.print_game        = true; }
        else if (strcmp(argv[i],"--print-solution")==0)
                                { options.print_solution    = true; }
        else if (strcmp(argv[i],"--print-statistics")==0)
                                { options.print_statistics  = true; }
        else if (strcmp(argv[i],"--verbose")==0)
                                { options.print_verbose     = true; }
        else if (strcmp(argv[i],"--flip")==0)
                                { options.flip              = true;}
        else if (strcmp(argv[i],"--parity")==0)
                                { options.win_conditions[0] = true; }
        else if (strcmp(argv[i],"--energy")==0)
                                { options.win_conditions[1] = true; }
        else if (strcmp(argv[i],"--mean-payoff")==0)
                                { options.win_conditions[2] = true; }

        else if (strcmp(argv[i],"--help")==0) {
            std::cout << "Usage: " << argv[0] << " [options]\n"
            << "Options:\n"
            << "  --dzn <filename>           : DZN file name\n"
            << "  --gm <filename>            : GM file name\n"
            << "  --jurd <levels> <blocks>   : Jurdzinski game\n"
            << "  --rand <ns> <ps> <d1> <d2> : Random game\n"
            << "  --mladder <bl>             : ModelcheckerLadder game\n"
            << "  --weights <w1> <w2>        : Weights range\n"
            << "  --weights-force <w1> <w2>  : Force to use weights range\n"
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
            << "  --chuffed                  : CP Solver (Chuffed)\n"
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
    switch (options.game_type) {
        case DZN: case GM:
            game = new Game(options.game_type, 
                            options.game_filename, 
                            options.weights,
                            options.init[0],
                            options.reward);
            break;
        case JURD: case RAND: case MLADDER:
            game = new Game(options.game_type, 
                            options.vals,
                            options.weights,
                            options.init[0],
                            options.reward);
            break;
        default:
            game = new Game({0,1},{3,2},{0,1},{1,0},{0,0},0,MAX);
            break;
    }
    double launchinggame = stopClock(); //...........................

    if (options.flip) game->flipGame();

    if (options.print_game || options.print_verbose) {
        game->printGame();
    }

    if ((options.print_time>1 || options.print_verbose)) {
        std::cout << "Game creation time : " << launchinggame << std::endl;
    }

    if (options.export_type == DZN) {
        game->exportFile(DZN, options.export_filename);
    }
    else if (options.export_type == GM) {
        game->exportFile(GM, options.export_filename);
    }
    else if (options.export_type == GMW) {
        game->exportFile(GMW, options.export_filename);
    }

    // Ensure at least one winning condition is selected, default --parity
    if (options.win_conditions[0]==false && 
        options.win_conditions[1]==false && 
        options.win_conditions[2]==false) 
    {
        options.win_conditions[0] = true;
    }

    //-------------------------------------------------------------------------
    // For testing purposes

    if (options.solver=="testing") {
    }

    //-------------------------------------------------------------------------
    // CP-NOC-Chuffed

    else if (options.solver.substr(0,3)=="noc"&&options.cpengine=="chuffed") {
        startClock(); //.............................................
        Chuffed::NOCModel* model = new Chuffed::NOCModel(
                            *game, options.win_conditions, options.threshold, 
                            (options.print_solution || options.print_verbose),
                            options.solver=="noc-even"?EVEN:ODD);

        so.print_sol = options.print_solution || options.print_verbose;
        double preptime = stopClock(); //............................

        if (options.print_time>1) {
            std::cout << "Init time          : " << preptime << std::endl;
        }

        startClock(); //.............................................
        if (options.print_solution || options.print_verbose) {
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

        if (options.print_time>1 || options.print_verbose) {
            std::cout << "Solving time       : " << totaltime << std::endl;
            std::cout << "Mem used           : " << memUsed() << std::endl;
        }

        if (options.solver=="noc-even") {
            if (options.print_time>=0 || options.print_verbose)
                std::cout   << game->init << ": " 
                            << (engine.solutions>0?"EVEN ":"ODD ");
        }
        else {
            if (options.print_time>=0 || options.print_verbose)
                std::cout   << game->init << ": " 
                            << (engine.solutions>0?"ODD ":"EVEN ");
        }

        if (options.print_time<=-2 || options.print_verbose) {
            std::cout   << preptime << "\t";
        }

        if (options.print_time!=0 || options.print_verbose) {
            std::cout   << totaltime;
        }        


        std::cout << std::endl;

        if (options.print_statistics || options.print_verbose) {
            engine.printStats();
        }
        
        delete model;
    }

    //-------------------------------------------------------------------------
    // CP-NOC-Gecode

    else if (options.solver.substr(0,3)=="noc"&&options.cpengine=="gecode") {

    #ifdef HAS_GECODE

        startClock(); //.............................................
        Gecode::NocModel* model = new Gecode::NocModel(
                            *game, options.win_conditions, options.threshold, 
                            options.solver=="noc-even"?EVEN:ODD);

        double preptime = stopClock(); //............................

        if (options.print_time>1) {
            std::cout << "Init time          : " << preptime << std::endl;
        }

        startClock(); //.............................................
        Gecode::DFS<Gecode::NocModel> dfs(model);
        delete model;
        Gecode::NocModel* solution = dfs.next();

        if (options.print_solution || options.print_verbose) {
            if (solution) solution->print();
            else std::cout << "UNSATISFIABLE" << std::endl;
        }
        double totaltime = stopClock(); //...........................

        if (options.print_time>1 || options.print_verbose) {
            std::cout << "Solving time       : " << totaltime << std::endl;
            std::cout << "Mem used           : " << "???" << std::endl;
        }

        if (options.solver=="noc-even") {
            if (options.print_time>=0 || options.print_verbose)
                std::cout << game->init << ": " << (solution?"EVEN ":"ODD ");
        }
        else {
            if (options.print_time>=0 || options.print_verbose)
                std::cout << game->init << ": " << (!solution?"ODD ":"EVEN ");
        }

        if (options.print_time<=-2 || options.print_verbose) {
            std::cout   << preptime << "\t";
        }

        if (options.print_time!=0 || options.print_verbose) {
            std::cout   << totaltime;
        }        

        std::cout << std::endl;

        if (options.print_statistics || options.print_verbose) {
            // engine.printStats();
            std::cout << "Statistics";
        }
        
        if (solution) delete solution;

    #else
        std::cout << "Error: Gecode support is disabled.\n" 
                  << "Please rebuild NOCQ using the --enable-gecode flag.\n";

    #endif //HAS_GECODE

    }

    //-------------------------------------------------------------------------
    // SAT Encoding

    else if (options.solver == "SAT") {
        SATEncoder encoder(*game);

        startClock(); //.............................................
        auto cnf = encoder.getCNF();
        double encodetime = stopClock(); //..........................

        startClock(); //.............................................
        encoder.dimacs(cnf,options.export_filename);
        double dimacstime = stopClock(); //..........................

        if (options.print_time>=0 || options.print_verbose) {
            std::cout << "Encoding time      : " << encodetime << std::endl;
            std::cout << "Dimacs time        : " << dimacstime << std::endl;
        } 
    }

    //-------------------------------------------------------------------------
    // ZRA

    else if (options.solver=="zra") {

        startClock(); //.............................................
        Zielonka zlk(*game);
        double preptime = stopClock(); //............................

        if (options.print_verbose) {
            std::cout << "Preparation time   : " << preptime << std::endl;
        }

        startClock(); //.............................................
        auto win = zlk.solve();
        double totaltime = stopClock(); //...........................

        if (options.print_solution || options.print_verbose) {
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

        for(auto& v0 : options.init) {
            auto it = std::find(win[0].begin(), win[0].end(), v0);

            if (options.print_time>=0 || options.print_verbose)
                std::cout << v0 << ": " << (it != win[0].end()?"EVEN ":"ODD ");
            
            
            if (options.print_time!=0 || options.print_verbose) {
                std::cout   << totaltime;
            }

            std::cout << std::endl;        
        }
    }

    //-------------------------------------------------------------------------
    // FRA

    else if (options.solver=="fra") {
        if (options.print_solution || options.print_verbose) {
            options.init.resize(game->nvertices);
            std::iota(options.init.begin(), options.init.end(), 0);
        }
        for(auto& v : options.init) {
            startClock(); //.............................................
            auto play = getPlay(*game, v, true);
            double totaltime = stopClock(); //...........................

            if (options.print_time>=0 || options.print_verbose)
                std::cout << v << ": " << (play==EVEN?"EVEN ":"ODD "); 

            if (options.print_time!=0 || options.print_verbose) {
                std::cout   << totaltime;
            }

            std::cout << std::endl;
        }
    }    

    //-------------------------------------------------------------------------
    // SCC

    else if (options.solver=="scc") { 
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