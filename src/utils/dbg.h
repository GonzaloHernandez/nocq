#ifndef DBG_H
#define DBG_H

#include <ostream>
#include <vector>
#include <sstream>

#ifndef modelling_h
#include "chuffed/vars/modelling.h"
#endif

//-----------------------------------------------------------------------------

inline std::string dbg_intvars(const vec<IntVar*>& obj) {
    std::stringstream ss;
    ss << "{";
    for (int i = 0; i < obj.size(); i++) {
        ss << (i?",":"");
        if ((*obj[i]).isFixed()) {
            ss << (*obj[i]).getVal();
        } else {
            ss << "[" <<(*obj[i]).getMin() << ".." <<(*obj[i]).getMax() << "]";
        }
    }
    ss << "}";
    return ss.str();
}

//-----------------------------------------------------------------------------

inline std::string dbg_boolviews(const vec<BoolView>& obj) {
    std::stringstream ss;
    ss << "{";
    for (int i = 0; i < obj.size(); i++) {
        if (!obj[i].isFixed())      ss << (i?",":"") << "?";
        else if (obj[i].isTrue())   ss << (i?",":"") << "+";
        else                        ss << (i?",":"") << "-";
    }
    ss << "}";
    return ss.str();
}

//-----------------------------------------------------------------------------

inline std::string dbg_ints(const vec<int>& obj) {
    std::stringstream ss;
    ss << "{";
    for (int i = 0; i < obj.size(); i++) {
        ss << (i?",":"") << obj[i];
    }
    ss << "}";
    return ss.str();
}

//-----------------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& os, const Lit& obj) {
    os << "L" << (!sign(obj)?"~":"") << var(obj);
    return os;
}

//-----------------------------------------------------------------------------

inline std::string dbg_lits(const vec<Lit>& obj) {
    std::stringstream ss;
    ss << "{";
    for (int i = 0; i < obj.size(); i++) {
        ss << (i?",":"") << obj[i];
    }
    ss << "}";
    return ss.str();
}

//-----------------------------------------------------------------------------

inline std::string dbg_clause(const Clause& obj) {
    std::stringstream ss;
    for (int i = 0; i < obj.sz; i++) {
        ss << (i?",":"");
        ss << obj[i];
    }
    ss << ")";

    return ss.str();
}

//-----------------------------------------------------------------------------

inline void launchdbg() {
    vec<IntVar*>    vs;
    vec<BoolView>   bs;
    vec<int>        is;
    vec<Lit>        ls;
    Clause*         c = Clause_new(ls);

    dbg_intvars(vs);
    dbg_boolviews(bs);
    dbg_ints(is);
    dbg_lits(ls);
    dbg_clause(*c);
}


#endif // DBG_H