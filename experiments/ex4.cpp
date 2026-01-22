#include "../various/fra.h"
#include "../various/game.h"
#include "../various/tarjan.h"
#include "../various/zielonka.h"
#include "../various/satencoder.h"

#include "../cp_nocq/nocq_chuffed.cpp"
#include "../cp_nocq/nocq_gecode.cpp"

#include "../resources/debugchuffed.h"
#include "../resources/debugstd.h"

//--------------------------------------------------------------------------------------

struct options {
    bool print_game         = false; 
    bool print_solution     = false; 
    bool print_statistics   = false; 
    bool print_verbose      = false; 
    int  print_time         = 0;    // 0=Default 1=Solving Time 2=All times
    int  game_type          = 0;    // enum game_type {DEF,JURD,RAND,MLADDER,DZN,GM,GMW,DIM}
    reward_type         reward          = MAX;
    std::vector<int>    vals            = {};
    std::vector<int>    weights         = {0,0,0};              // lbound ubound force
    std::vector<int>    starts          = {0};
    std::string         game_filename   = "";
    std::string         export_filename = "";
    int                 export_type     = 0;                    // 0=not DZN GM GMW DIM
    std::string         solver          = "";                   // CP-NOC-EVEN / CP-NOC-ODD / SAT-zchaff / SAT-cadical / ZRA / FRA / SCC
    std::string         cpengine        = "chuffed";            // chuffed / gecode
    int                 proof           = 0;                    // 0=no 1=yes 2=eager
    int                 flip            = 0;                    // 0=no 1=flip 2=flip_to_compare
    int                 threshold       = 1;                    // threshold for flip_to_compare
    std::vector<bool>   win_conditions  = {false,false,false};  // 0=parity 1=energy 2=mean-payoff
} options;

//--------------------------------------------------------------------------------------

bool parseMyOptions(int argc, char *argv[]) {
    so.nof_solutions = 1;
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i],"--jurd")==0) {
            options.game_type = JURD;
            i++;
            
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Number of levels missing\n";
                return false;                    
            }
            char* endptr;
            int levels = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || levels < 2 || levels > 1000000) {
                std::cerr << "ERROR: Jurdzinski level out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Jurdzinski level no numeric\n";
                return false;
            }
            options.vals.push_back(levels);

            i++;
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Number of blocks missing\n";
                return false;                    
            }
            int blocks = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || blocks < 1 || blocks > 1000000) {
                std::cerr << "ERROR: Jurdzinski blocks out of range\n";
                return false;
            }            
            if (*endptr != '\0') {
                std::cerr << "ERROR: Jurdzinski blocks no numeric\n";
                return false;
            }
            options.vals.push_back(blocks);
        }
        else if (strcmp(argv[i],"--rand")==0) {
            options.game_type = RAND;
            i++;
            
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Number of vertices missing\n";
                return false;                    
            }
            char* endptr;
            int ns = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || ns < 1 || ns > 10000000) {
                std::cerr << "ERROR: Number of vertices out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Number of vertices no numeric\n";
                return false;
            }
            options.vals.push_back(ns);

            i++;
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Number of priorities missing\n";
                return false;                    
            }
            int ps = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || ps < 1 || ps > 10000000) {
                std::cerr << "ERROR: Amount of priorities out of range\n";
                return false;
            }            
            if (*endptr != '\0') {
                std::cerr << "ERROR: Amount of priorities no numeric\n";
                return false;
            }
            options.vals.push_back(ps);

            i++;
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Min amount of edges missing\n";
                return false;                    
            }
            int d1 = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || d1 < 1 || d1 > 199) {
                std::cerr << "ERROR: Min amount of edges out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Min amount of edges  no numeric\n";
                return false;
            }
            options.vals.push_back(d1);

            i++;
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Max amount of edges missing\n";
                return false;                    
            }
            int d2 = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || d2 < 2 || d2 > 300) {
                std::cerr << "ERROR: Max amount of edges out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Max amount of edges  no numeric\n";
                return false;
            }
            options.vals.push_back(d2);
        }
        else if (strcmp(argv[i],"--mladder")==0) {
            options.game_type = MLADDER;
            i++;
            
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Number of blocks missing\n";
                return false;                    
            }
            char* endptr;
            int blocks = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || blocks < 1 || blocks > 1000000) {
                std::cerr << "ERROR: MLadder blocks out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: MLadder blocks no numeric\n";
                return false;
            }
            options.vals.push_back(blocks);
        }
        else if (strncmp(argv[i],"--weights",9)==0) {
            if (strcmp(argv[i],"--weights-force")==0) options.weights[2] = 1;
            i++;
            
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Initial weight missing\n";
                return false;                    
            }
            char* endptr;
            int w1 = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || w1 < -1000000 || w1 > 1000000) {
                std::cerr << "ERROR: Weights out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Initial weight no numeric\n";
                return false;
            }
            options.weights[0] = w1;

            i++;
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Final weight missing\n";
                return false;                    
            }
            int w2 = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || w2 < w1 || w2 > 1000000) {
                std::cerr << "ERROR: Weights out of range\n";
                return false;
            }            
            if (*endptr != '\0') {
                std::cerr << "ERROR: Final weight no numeric\n";
                return false;
            }
            options.weights[1] = w2;
        }
        else if (strcmp(argv[i],"--dzn")==0) {
            options.game_type = DZN;
            i++;
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: DZN file name missing\n";
                return false;                    
            }
            options.game_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--gm")==0) {
            options.game_type = GM;
            i++;
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: GM file name missing\n";
                return false;                    
            }
            options.game_filename = argv[i];                
        }        
        else if (strcmp(argv[i],"--dimacs")==0) {
            options.game_type = DIM;
            i++;
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Dimacs file name missing\n";
                return false;                    
            }
            options.game_filename = argv[i];
        }
        else if (strcmp(argv[i],"--start")==0) {
            i++;
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Starting vertex missing\n";
                return false;                    
            }

            options.starts.clear();
            std::string s = argv[i];
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, ',')) {
                size_t start = item.find_first_not_of(" \t");
                size_t end = item.find_last_not_of(" \t");

                if (start == std::string::npos || end == std::string::npos) {
                    std::cerr << "ERROR: Empty or invalid element found between commas\n";
                    return false;
                }

                options.starts.push_back(std::stoi(item));
            }
        }
        else if (strcmp(argv[i],"--export-dzn")==0) {
            i++;
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Target DZN file name missing\n";
                return false;                    
            }
            options.export_type = DZN;
            options.export_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--export-gm")==0) {
            i++;
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Target GM file name missing\n";
                return false;                    
            }
            options.export_type = GM;
            options.export_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--export-gmw")==0) {
            i++;
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Target GMW file name missing\n";
                return false;                    
            }
            options.export_type = GMW;
            options.export_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--export-dimacs")==0) {
            i++;
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Target DIMAS file name missing\n";
                return false;                    
            }
            options.export_type = DIM;
            options.export_filename = argv[i];        
        }
        else if (strcmp(argv[i],"--nsolutions")==0) {
            i++;
            
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Number of solutions missing\n";
                return false;                    
            }
            char* endptr;
            int ns = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || ns < 0 || ns > 10) {
                std::cerr << "ERROR: Number of solutions out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: MLadder blocks no numeric\n";
                return false;
            }
            so.nof_solutions = ns;
        }
        else if (strcmp(argv[i],"--threshold")==0) {
            i++;
            
            if (i>=argc || (strlen(argv[i]) > 1 && strncmp(argv[i], "--", 2) == 0)) {
                std::cerr << "ERROR: Threshold value missing\n";
                return false;                    
            }
            char* endptr;
            int th = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || th < -1000000 || th > 1000000) {
                std::cerr << "ERROR: Threshold value out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Threshold value not numeric\n";
                return false;
            }
            options.threshold = th;
        }

        else if (strcmp(argv[i],"--max")==0)                { options.reward = MAX; }
        else if (strcmp(argv[i],"--min")==0)                { options.reward = MIN; }

        else if (strcmp(argv[i],"--testing")==0)            { options.solver = "testing"; }
        else if (strcmp(argv[i],"--noc")==0)                { options.solver = "noc-even"; }
        else if (strcmp(argv[i],"--noc-even")==0)           { options.solver = "noc-even"; }
        else if (strcmp(argv[i],"--noc-odd")==0)            { options.solver = "noc-odd"; }
        else if (strcmp(argv[i],"--zchaff")==0)             { options.solver = "zchaff"; }
        else if (strcmp(argv[i],"--cadical")==0)            { options.solver = "cadical"; }
        else if (strcmp(argv[i],"--zra")==0)                { options.solver = "zra"; }
        else if (strcmp(argv[i],"--fra")==0)                { options.solver = "fra"; }
        else if (strcmp(argv[i],"--scc")==0)                { options.solver = "scc"; }
        else if (strcmp(argv[i],"--chuffed")==0)            { options.cpengine = "chuffed"; }
        else if (strcmp(argv[i],"--gecode")==0)             { options.cpengine = "gecode"; }

        else if (strcmp(argv[i],"--proof")==0)              { options.proof = 1; }
        else if (strcmp(argv[i],"--proof-eager")==0)        { options.proof = 2; }
        else if (strcmp(argv[i],"--print-only-times")==0)   { options.print_time        = -2; }
        else if (strcmp(argv[i],"--print-only-time")==0)    { options.print_time        = -1; }
        else if (strcmp(argv[i],"--print-time")==0)         { options.print_time        = 1; }
        else if (strcmp(argv[i],"--print-times")==0)        { options.print_time        = 2; }
        else if (strcmp(argv[i],"--print-game")==0)         { options.print_game        = true; }
        else if (strcmp(argv[i],"--print-solution")==0)     { options.print_solution    = true; }
        else if (strcmp(argv[i],"--print-statistics")==0)   { options.print_statistics  = true; }
        else if (strcmp(argv[i],"--verbose")==0)            { options.print_verbose     = true; }

        else if (strcmp(argv[i],"--flip")==0)               { options.flip  = 1;}
        else if (strcmp(argv[i],"--flip-compare")==0)       { options.flip  = 2;}

        else if (strcmp(argv[i],"--parity")==0)             { options.win_conditions[0] = true; }
        else if (strcmp(argv[i],"--energy")==0)             { options.win_conditions[1] = true; }
        else if (strcmp(argv[i],"--mean-payoff")==0)        { options.win_conditions[2] = true; }

        else if (strcmp(argv[i],"--help")==0) {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --dzn <filename>           : DZN file name\n";
            std::cout << "  --gm <filename>            : GM file name\n";
            std::cout << "  --jurd <levels> <blocks>   : Jurdzinski game\n";
            std::cout << "  --rand <ns> <ps> <d1> <d2> : Random game\n";
            std::cout << "  --mladder <bl>             : ModelcheckerLadder game\n";
            std::cout << "  --weights <w1> <w2>        : Weights range\n";
            std::cout << "  --weights-force <w1> <w2>  : Force to use weights range\n";
            std::cout << "  --start <vertex>           : Starting vertex\n";
            std::cout << "  --print-only-time          : Print only solving time\n";
            std::cout << "  --print-only-times         : Print only preptime + solving time\n";
            std::cout << "  --print-time               : Print solving time\n";
            std::cout << "  --print-times              : Print all times\n";
            std::cout << "  --print-game               : Print game\n";
            std::cout << "  --print-solution           : Print solution (All vertices)\n";
            std::cout << "  --print-statistics         : Print statistics after solving\n";
            std::cout << "  --verbose                  : Print everything\n";
            std::cout << "  --max                      : Seek to maximize the priority\n";
            std::cout << "  --min                      : Seek to minimize the priority\n";
            std::cout << "  --export-dzn <filename>    : Export game to DZN format (not solve)\n";
            std::cout << "  --export-gm <filename>     : Export game to GM format (not solve)\n"; 
            std::cout << "  --export-gmw <filename>    : Export game to GM+Weights format (not solve)\n"; 
            std::cout << "  --export-dimacs <filename> : Export game to DIMACS format (not solve)\n"; 
            std::cout << "  --noc-even                 : CP-NOC satisfying player EVEN (No-Odd-Cycles)\n";
            std::cout << "  --noc-odd                  : CP-NOC satisfying player ODD (No-Even-Cycles)\n";
            std::cout << "  --chuffed                  : CP Solver (Chuffed)\n";
            std::cout << "  --gecode                   : CP Solver (Gecode)\n";
            std::cout << "  --fra                      : Solve using FRA\n";
            std::cout << "  --sat-encoding             : Encode on DIMACS\n";
            std::cout << "  --sat-zchaff               : Solve using zChaff\n";
            std::cout << "  --sat-cadical              : Solve using Cadical\n";
            std::cout << "  --proof                    : Compare results using Zielonka\n";
            std::cout << "  --proof-eager              : Looking for counterexample\n";
            std::cout << "  --checker-scc              : Checker by CP-NOC\n";
            std::cout << "  --checker-dfs-recursive    : Checker by DFS Recursive\n";
            std::cout << "  --checker-dfs-iterative    : Checker by DFS Iterative\n";
            std::cout << "  --flip                     : Convert the game into its complement\n";
            std::cout << "  --flip-compare             : Solve showing dualitity converting the game into its complement \n";
            std::cout << "  --threshold <value>        : Threshold for flip-compare (default=1)\n";
            std::cout << "  --parity                   : Parity condition (default)\n";
            std::cout << "  --energy                   : Energy condition\n";
            std::cout << "  --mean-payoff              : Mean-Payoff condition\n";
            return false;
        }
        else {
            std::cerr << "ERROR: Unknown option: " << argv[i] << std::endl;
            return false;
        }
    }
    return true;
}

//======================================================================================

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    using PcloseFunc = int (*)(FILE*);
    std::unique_ptr<FILE, PcloseFunc> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

//======================================================================================

int main(int argc, char *argv[])
{
    launchdebugstd();
    launchdebugchuffed();
    if (!parseMyOptions(argc, argv)) exit(1);
    Game* game = nullptr;

    //----------------------------------------------------------------------------------

    auto start = std::chrono::high_resolution_clock::now();

    switch (options.game_type) {
        case DZN: case GM:
            game = new Game(options.game_type, 
                            options.game_filename, 
                            options.weights,
                            options.starts[0],
                            options.reward);
            break;
        case JURD: case RAND: case MLADDER:
            game = new Game(options.game_type, 
                            options.vals,
                            options.weights,
                            options.starts[0],
                            options.reward);
            break;
        case DIM:
            break;
        default:
            game = new Game({0,1},{3,2},{0,1},{1,0},EVEN,MIN);
            break;
    }

    if (options.flip == 1) game->flipGame();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> launchinggame = end - start;

    if (game && (options.print_game || options.print_verbose) && options.proof<2) {
        game->printGame();
    }

    if ((options.print_time>1 || options.print_verbose) && options.proof<2) {
        std::cout << "Game creation time : " << launchinggame.count() << std::endl;
    }

    if (game && options.export_type == DZN && options.proof<2) {
        game->exportFile(DZN, options.export_filename);
    }
    else if (game && options.export_type == GM && options.proof<2) {
        game->exportFile(GM, options.export_filename);
    }
    else if (game && options.export_type == GMW && options.proof<2) {
        game->exportFile(GMW, options.export_filename);
    }
    else if (game && options.export_type == DIM && options.proof<2) {
        SATEncoder encoder(*game);

        auto start = std::chrono::high_resolution_clock::now();
        auto cnf = encoder.getCNF();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> encodetime = end - start;

        start = std::chrono::high_resolution_clock::now();
        encoder.dimacs(cnf,options.export_filename);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> dimacstime = end - start;

        if (options.print_time>1 || options.print_verbose) {
            std::cout << "Encoding time      : " << encodetime.count() << std::endl;
            std::cout << "Dimacs time        : " << dimacstime.count() << std::endl;
        } 
    }

    // Ensure at least one winning condition is selected otherwise default to parity
    if (options.win_conditions[0]==false && 
        options.win_conditions[1]==false && 
        options.win_conditions[2]==false) 
    {
        options.win_conditions[0] = true;
    }

    //----------------------------------------------------------------------------------
    // For testing purposes
    if (options.solver=="testing") {
    }

    //----------------------------------------------------------------------------------
    // CP-NOC-Chuffed

    else if (game && options.proof==0 && 
        options.solver.substr(0, 3) == "noc" && options.cpengine=="chuffed") 
    {
        start = std::chrono::high_resolution_clock::now();
        NOCModel* model = new NOCModel(
                                *game, options.win_conditions, options.threshold, 
                                (options.print_solution || options.print_verbose),
                                options.solver=="noc-even"?EVEN:ODD);

        so.print_sol = options.print_solution || options.print_verbose;
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> preptime = end - start;            

        if (options.print_time>1) {
            std::cout << "Init time          : " << preptime.count() << std::endl;
        }

        start = std::chrono::high_resolution_clock::now();

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

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totaltime = end - start;
        
        if (options.print_time>1 || options.print_verbose) {
            std::cout << "Solving time       : " << totaltime.count() << std::endl;
            std::cout << "Mem used           : " << memUsed() << std::endl;
        }

        if (options.solver=="noc-even") {
            if (options.print_time>=0 || options.print_verbose)
                std::cout << game->start << ": " << (engine.solutions>0?"EVEN ":"ODD ");
        }
        else {
            if (options.print_time>=0 || options.print_verbose)
                std::cout << game->start << ": " << (engine.solutions>0?"ODD ":"EVEN ");
        }

        if (options.print_time<=-2 || options.print_verbose) {
            std::cout   << preptime.count() << "\t";
        }

        if (options.print_time!=0 || options.print_verbose) {
            std::cout   << totaltime.count();
        }        


        std::cout << std::endl;

        if (options.print_statistics || options.print_verbose) {
            engine.printStats();
        }
        
        delete model;
    }

    //----------------------------------------------------------------------------------
    // CP-NOC-Gecode

    else if (game && options.proof==0 && 
        options.solver.substr(0, 3) == "noc" && options.cpengine=="gecode") 
    {
        start = std::chrono::high_resolution_clock::now();
        NocModelGecode* model = new NocModelGecode(
                                *game, options.win_conditions, options.threshold, 
                                options.solver=="noc-even"?EVEN:ODD);

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> preptime = end - start;            

        if (options.print_time>1) {
            std::cout << "Init time          : " << preptime.count() << std::endl;
        }

        start = std::chrono::high_resolution_clock::now();

        Gecode::DFS<NocModelGecode> dfs(model);
        delete model;
        NocModelGecode* solution = dfs.next();

        if (options.print_solution || options.print_verbose) {
            if (solution) solution->print();
            else std::cout << "UNSATISFIABLE" << std::endl;
        }

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totaltime = end - start;

        if (options.print_time>1 || options.print_verbose) {
            std::cout << "Solving time       : " << totaltime.count() << std::endl;
            std::cout << "Mem used           : " << "???" << std::endl;
        }

        if (options.solver=="noc-even") {
            if (options.print_time>=0 || options.print_verbose)
                std::cout << game->start << ": " << (solution?"EVEN ":"ODD ");
        }
        else {
            if (options.print_time>=0 || options.print_verbose)
                std::cout << game->start << ": " << (!solution?"ODD ":"EVEN ");
        }

        if (options.print_time<=-2 || options.print_verbose) {
            std::cout   << preptime.count() << "\t";
        }

        if (options.print_time!=0 || options.print_verbose) {
            std::cout   << totaltime.count();
        }        

        std::cout << std::endl;

        if (options.print_statistics || options.print_verbose) {
            // engine.printStats();
            std::cout << "Statistics";
        }
        
        if (solution) delete solution;
        
    }

    //----------------------------------------------------------------------------------
    // SAT-zchaff

    else if (options.proof==0 && options.solver=="zchaff") {
        std::string output;
        if (options.game_type == DIM) {
            std::string command =   "./zchaff " + options.game_filename + 
                                    " | grep -E \"RESULT|Total Run Time\"";
            output = exec(command.c_str());
        }
        else {
            SATEncoder encoder(*game);
            auto start = std::chrono::high_resolution_clock::now();
            auto cnf = encoder.getCNF();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> encodetime = end - start;
            
            start = std::chrono::high_resolution_clock::now();
            encoder.dimacs(cnf,"temp.cnf");
            end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> dimacstime = end - start;
            
            output = exec("./zchaff temp.cnf | grep -E \"RESULT|Total Run Time\"");
            std::cout << game->nvertices << " ";
            std::cout << game->nedges << " ";
            std::cout << encodetime.count() << " ";
            std::cout << dimacstime.count() << " ";
        }
    
        std::cout << output << std::endl;
     }

    //----------------------------------------------------------------------------------
    // SAT-Cadical

    else if (options.proof==0 && options.solver=="cadical") {
        std::string output;
        if (options.game_type == DIM) {
            std::string command =   "./cadical " + options.game_filename + 
                                    " | grep -E \"^s |total process time since initialization\"";
            output = exec(command.c_str());
        }
        else {
            SATEncoder encoder(*game);
            auto start = std::chrono::high_resolution_clock::now();
            auto cnf = encoder.getCNF();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> encodetime = end - start;
            
            start = std::chrono::high_resolution_clock::now();
            encoder.dimacs(cnf,"temp.cnf");
            end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> dimacstime = end - start;    

            output = exec("./cadical temp.cnf | grep -E \"^s |total process time since initialization\"");
            std::cout << game->nvertices << " ";
            std::cout << game->nedges << " ";
            std::cout << encodetime.count() << " ";
            std::cout << dimacstime.count() << " ";
        }

        std::cout << output << std::endl;
    }

    //----------------------------------------------------------------------------------
    // ZRA

    else if (game && options.proof==0 && options.solver=="zra") {
        for (int f=0; f<=1; f++) {
            start = std::chrono::high_resolution_clock::now();

            Zielonka zlk(*game);
            
            end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> preptime = end - start;

            if (options.print_verbose) {
                std::cout << "Preparation time   : " << preptime.count() << std::endl;
            }

            start = std::chrono::high_resolution_clock::now();
            auto win = zlk.solve();
            end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> totaltime = end - start;

            if (options.print_solution || options.print_verbose) {
                std::cout << "EVEN " << win[0] << std::endl;
                std::cout << "ODD  " << win[1] << std::endl;
            }

            for(auto& v0 : options.starts) {
                auto it = std::find(win[0].begin(), win[0].end(), v0);

                if (options.print_time>=0 || options.print_verbose)
                    std::cout << v0 << ": " << (it != win[0].end()?"EVEN ":"ODD ");
                
                
                if (options.print_time!=0 || options.print_verbose) {
                    std::cout   << totaltime.count();
                }

                std::cout << std::endl;
            
            }
            if (options.flip < 2) // not compare with flipped game
                break;
            game->flipGame();
        }
    }

    //----------------------------------------------------------------------------------
    // FRA

    else if (game && options.proof==0 && options.solver=="fra") {
        if (options.print_solution || options.print_verbose) {
            options.starts.resize(game->nvertices);
            std::iota(options.starts.begin(), options.starts.end(), 0);
        }
        for(auto& v : options.starts) {
            start = std::chrono::high_resolution_clock::now();

            auto play = getPlay(*game, v, true);

            end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> totaltime = end - start;

            if (options.print_time>=0 || options.print_verbose)
                std::cout << v << ": " << (play==EVEN?"EVEN ":"ODD "); 

            if (options.print_time!=0 || options.print_verbose) {
                std::cout   << totaltime.count();
            }

            std::cout << std::endl;
        }
    }    

    //----------------------------------------------------------------------------------
    // SCC

    else if (game && options.proof==0 && options.solver=="scc") { 
        GameView view(*game);
        TarjanSCC tscc(*game,view);
        auto sccs = tscc.solve();
        for (auto& scc : sccs) {
            std::cout << scc << std::endl;            
        }
    }

    //==================================================================================
    //CP-NOC + Zielonka (proof)

    else if (game && options.proof==1 && (options.solver=="noc-even" || options.solver=="noc-odd")) { 
        start = std::chrono::high_resolution_clock::now();
        Zielonka zlk(*game);

        NOCModel* model = new NOCModel(   
                                *game, options.win_conditions, options.threshold, 
                                (options.print_solution || options.print_verbose),
                                options.solver=="noc-even"?EVEN:ODD);

        so.print_sol = options.print_solution || options.print_verbose;

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> preptime = end - start;

        if (options.print_time>1 || options.print_verbose) {
            std::cout << "Preparation time   : " << preptime.count() << std::endl;
        }

        start = std::chrono::high_resolution_clock::now();
        auto win = zlk.solve();
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totaltime1 = end - start;
        
        if (options.print_time>1 || options.print_verbose) {
            std::cout << "Solving time (ZRA) : " << totaltime1.count() << std::endl;
        }

        start = std::chrono::high_resolution_clock::now();

        std::streambuf* old_buf = std::cout.rdbuf();
        std::ofstream null_stream("/dev/null");  // or "NUL" on Windows
        std::cout.rdbuf(null_stream.rdbuf());

        engine.solve(model);

        std::cout.rdbuf(old_buf);

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totaltime2 = end - start;

        if (options.print_time>1 || options.print_verbose) {
            std::cout << "Solving time (CP)  : " << totaltime2.count() << std::endl;
        }

        if (((engine.solutions>0 && options.solver=="noc-even") || (engine.solutions==0 && options.solver=="noc-odd")) && 
            std::find(win[0].begin(), win[0].end(), game->start) != win[0].end()) 
        {
            std::cout << ".";
        }
        else if (((engine.solutions==0 && options.solver=="noc-even") || (engine.solutions>0 && options.solver=="noc-odd")) && 
            std::find(win[1].begin(), win[1].end(), game->start) != win[1].end()) 
        {
            std::cout << ".";
        }
        else {
            std::cout << "Counter example found (Starting at " << game->start << "):" << std::endl;
            if (options.export_type == DZN) {
                game->exportFile(DZN, options.export_filename);
            }
            else if (options.export_type == GM) {
                game->exportFile(GM, options.export_filename);
            }

            if (options.print_game || options.print_verbose) {
                game->printGame();
            }            
        }

        delete model;
    }

    //----------------------------------------------------------------------------------

    delete game;

    return 0;
}


