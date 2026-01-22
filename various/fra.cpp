#include "fra.h"

struct splay {
    int         loop;
    long long   best;
    int         from;
    bool touched()  { return loop>=0; }
    bool parity()   { return best%2; }
};

//-----------------------------------------------------------------------------------------------------

std::vector<int> operator+(const std::vector<int>& v, int x) {
    std::vector<int> result = v;
    result.push_back(x);
    return result;
}

std::ostream& operator<<(std::ostream& os, const std::vector<int>& vec) {
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i];
        if (i != vec.size() - 1)
            os << ", ";
    }
    os << "]";
    return os;
}

//-----------------------------------------------------------------------------------------------------

int findVertex(int vertex,const std::vector<int>& path) {
    for (int i=0; i<path.size(); i++) {
        if (path[i] == vertex) return i;
    }
    return -1;
}

//-----------------------------------------------------------------------------------------------------

int findVertexReverse(int vertex,const std::vector<int>& path) {
    for (int i=path.size()-1; i>=0; i--) {
        if (path[i] == vertex) return i;
    }
    return -1;
}

//-----------------------------------------------------------------------------------------------------

bool isBetter(Game& g,int v1,int v2) {
    if ((g.reward==MIN && v1 < v2) || (g.reward==MAX && v1 > v2)) {
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------------------------

long long bestcolor(Game& g, int index,const std::vector<int>& path){
    long long m = g.priors[path[index]];
    for (int i=index+1; i<path.size(); i++) {
        if ((g.reward==MIN && g.priors[path[i]] < m) || (g.reward==MAX && g.priors[path[i]] > m)) {
            m = g.priors[path[i]];
        }
    }
    return m;
}

//-----------------------------------------------------------------------------------------------------

signed char getPlayBasic(Game& g, std::vector<int> path, int v) {
    int index = findVertex(v,path);
    if (index >= 0) {
        int best = bestcolor(g,index,path);
        return best%2;
    }
    else {
        int p = g.owners[v];
        for(auto& e : g.outs[v]) {
            std::vector <int> newpath = path;
            newpath.push_back(v);
            auto next = getPlayBasic(g, newpath, g.targets[e]);
            if (next == p) {
                return p;
            }
        }
        return 1-p;
    }
}

//-----------------------------------------------------------------------------------------------------

signed char getPlay(Game& g, int start, bool basic) {
    return getPlayBasic(g, {}, start);
}

//-----------------------------------------------------------------------------------------------------

bool getAllCycles(Game& g, std::vector<int> path, int v, std::vector<bool>& touched) {
    int index = findVertex(v,path);
    if (index >= 0) {
        int best = bestcolor(g,index,path);
        std::cout << path+v << std::endl;
        return best%2;
    }
    else {
        int p = g.owners[v];
        for(auto& e : g.outs[v]) {
            if (touched[e]) {
                std::cout<< path+v << " *" << std::endl;
                continue;
            }
            auto next = getAllCycles(g, path+v, g.targets[e], touched);
            touched[e] = true;
            // if (next == p) {
            //     return p;
            // }
        }
        return 1-p;
    }
}