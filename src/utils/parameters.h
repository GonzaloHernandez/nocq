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
#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <cstring>

#ifndef GAME_H
#include "game.h"
#endif

//-----------------------------------------------------------------------------

struct options {
    bool printGame          = false; 
    bool printSolution      = false; 
    bool printStatistics    = false; 
    bool printVerbose       = false; 
    int  printTime          = 0;        // 0=Default 1=Solving Time 2=All-times
    game_type  gameType     = DEF;

    objective_type  objective       = MAX;      // MAXimize,MINimize
    vec<int32_t>    vals;
    int64_t         lbound          = 0;
    int64_t         ubound          = 0;
    vec<int32_t>    init;
    std::string     gameFilename    = "";
    std::string     exportFilename  = "";
    game_type       exportType      = DEF;      // DZN,GM,GMW,GAME,DIM
    std::string     method          = "";       // noc-even,noc-odd,sat
                                                // zra,fra,scc

    std::string     solver          = "";       // chuffed-bool
                                                // chuffed-int
                                                // gecode, cadical

    bool            flip            = false;
    bool            parityCond      = false;
    bool            energyCond      = false;
    bool            meanpayoffCond  = false;
    int64_t         thresholdEnergy = 0;
    double          thresholdMPG    = 0.0;
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
    auto parseInteger = [&](const char* str, int64_t min, int64_t max) -> int {
        char* endptr;
        errno = 0;
        int64_t val = std::strtoll(str, &endptr, 10);

        if (errno == ERANGE || val < min || val > max) {
            std::cerr   << "ERROR: Value [" << str << "] out of range (" 
                        << min << "-" << max << ")\n";
            exit(1);
        }
        if (*endptr != '\0') {
            std::cerr<< "ERROR: Value [" << str << "] is not a valid number\n";
            exit(1);
        }
        return (int64_t)val;
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
    auto parseDouble = [&](const char* str, double min, double max) -> double {
        char* endptr;
        errno = 0;
        double val = std::strtod(str, &endptr); 
        
        if (errno == ERANGE) {
            std::cerr << "ERROR: Value [" << str << "] out of range\n";
            exit(1);
        }
        
        if (val < min || val > max) {
            std::cerr << "ERROR: Value [" << str << "] out of bounds (" 
                    << min << "-" << max << ")\n";
            exit(1);
        }

        if (*endptr != '\0') {
            const char* check = endptr;
            while (std::isspace(static_cast<unsigned char>(*check))) check++;
            if (*check != '\0') {
                std::cerr << "ERROR: Value [" << str << "] contains invalid characters\n";
                exit(1);
            }
        }        
        return val;
    };
    //-------------------------------------------------------------------------
    auto showHelp = [&]() {
        std::cout << "Usage: " << argv[0] << " [options]\n"
        << "\n"
        << "Game creation:\n"
        << "  --dzn <filename>           : Load DZN file\n"
        << "  --gm <filename>            : Load GM file\n"
        << "  --jurd <levels> <blocks>   : Jurdzinski game\n"
        << "  --rand <ns> <ps> <d1> <d2> : Random game\n"
        << "  --mladder <bl>             : ModelcheckerLadder game\n"
        << "  --sprand <vs> <density>    : Random SPRAND game\n"
        << "  --sqnc <size> <type>       : Structured synthetic game\n"
        << "\n"
        << "Global Settings:\n"
        << "  --init <vertex>            : Initial vertex (Default=0)\n"
        << "  --max | --min              : Optimization goal (Default: --max)\n"
        << "  --weights <w1> <w2>        : Weights range\n"
        // << "  --flip                     : Complement the game\n"
        << "\n"
        << "Methods:\n"
        << "  --noc-even | --noc-odd     : NOC player preference (Default: --noc-even)\n"
        << "  --fra                      : Solve using FRA algorithm\n"
        << "  --scc                      : Compute Strongly Connected Components\n"
        << "  --sat-encoding <filename>  : Encode on DIMACS file\n"
        << "\n"
        << "Solvers:\n"
        << "  --chuffed-bool             : Use Chuffed with BoolVars (Default)\n"
        << "  --chuffed-int              : Use Chuffed with IntVars\n"
        << "  --gecode                   : Use Gecode solver (BoolVars)\n"
        << "  --cadical                  : Use Cadical solver\n"
        << "\n"
        << "Conditions:\n"
        << "  --parity                   : Parity condition (default)\n"
        << "  --energy [thresh]          : Energy condition (default Threshold=0)\n"
        << "  --mean-payoff [thresh]     : Mean-Payoff condition (default Threshold=0.0)\n"
        << "\n"
        << "Output & Export:\n"
        << "  --print-time               : Print result + solving time\n"
        << "  --print-times              : Print result + all times\n"
        << "  --print-only-time          : Print solving time (no result)\n"
        << "  --print-only-times         : Print all times (no result)\n"
        << "  --print-game               : Print game\n"
        << "  --print-solution           : Print solution\n"
        << "  --print-statistics         : Print statistics after solving\n"
        << "  --verbose                  : Print everything\n"
        << "  --export-dzn <filename>    : Export game to DZN format\n"
        << "  --export-gm <filename>     : Export game to GM format\n"
        << "  --export-gmw <filename>    : Export game to GM + Weights\n"
        << "  --export-chpka <filename>  : Export Energy game (Chaolupka)\n"
        << "";
        exit(0);

    };
    //-------------------------------------------------------------------------
    if (argc==1) showHelp();
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
        else if (strcmp(argv[i],"--sprand")==0) {
            validateArg("--sprand <vertices>");
            options.vals.push(parseInteger(argv[i], 1, 10000000));
            validateArg("--sprand <density>");
            options.vals.push(parseInteger(argv[i], 1, 1000));
            options.gameType = SPRAND;
        }
        else if (strcmp(argv[i],"--sqnc")==0) {
            validateArg("--sqnc <size> <type>");
            options.vals.push(parseInteger(argv[i], 1, 100000));
            validateArg("--sqnc <type>");
            options.vals.push(parseInteger(argv[i], 1, 5));
            options.gameType = SQNC;
        }
        else if (strcmp(argv[i],"--weights")==0) {
            validateArg("--weights <lower_bound upper_bound>");
            options.lbound = parseInteger(argv[i], -1000000, 1000000);
            validateArg("--weights <final_weight>");
            options.ubound = parseInteger(argv[i], -1000000, 1000000);
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
        else if (strcmp(argv[i],"--export-chpka")==0) {
            validateArg("--export-chpka <filename>");
            options.exportType = CHPKA;
            options.exportFilename = argv[i];                
        }
        else if (strcmp(argv[i],"--sat-encoding")==0) {
            validateArg("--sat-encoding <filename>");
            options.method = "sat";
            options.exportType = DIM;
            options.exportFilename = argv[i];
        }
        else if (strcmp(argv[i],"--nsolutions")==0) {
            validateArg("--nsolutions <number>");
            so.nof_solutions = parseInteger(argv[i], 0, 10);
        }
        else if (strcmp(argv[i],"--energy")==0) {
            i++; // Move to the next argument
            if (i>=argc || (strlen(argv[i])>1 && strncmp(argv[i],"--",2) == 0)) {
                options.thresholdEnergy = 0;
                i--;
            } else {
                options.thresholdEnergy = parseInteger(argv[i], LLONG_MIN, LLONG_MAX);
            }
            options.energyCond = true;
        }
        else if (strcmp(argv[i],"--mean-payoff")==0) {
            i++; // Move to the next argument
            if (i>=argc || (strlen(argv[i])>1 && strncmp(argv[i],"--",2) == 0)) {
                options.thresholdMPG = 0.0;
                i--;
            } else {
                options.thresholdMPG = parseDouble(argv[i], -1e15, 1e15);
            }
            options.meanpayoffCond = true;
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
                                { options.solver          = "chuffed-bool"; }
        else if (strcmp(argv[i],"--chuffed-bool")==0)
                                { options.solver          = "chuffed-bool"; }
        else if (strcmp(argv[i],"--chuffed-int")==0)
                                { options.solver          = "chuffed-int"; }
        else if (strcmp(argv[i],"--gecode")==0)
                                { options.solver          = "gecode"; }
        else if (strcmp(argv[i],"--cadical")==0)
                                { options.solver          = "cadical"; }
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

        else if (strcmp(argv[i],"--help")==0) {
            showHelp();
        }
        else {
            std::cerr << "ERROR: Unknown option: " << argv[i] << std::endl;
            exit(0);
        }
    }
    return true;
}

//-----------------------------------------------------------------------------

#endif // PARAMETERS_H