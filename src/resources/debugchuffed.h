#ifndef debugchuffed_h
#define debugchuffed_h

#include <ostream>
#include <vector>
#include <sstream>

#ifndef modelling_h
#include "chuffed/vars/modelling.h"
#endif

// --------------------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& os, const BoolView& obj) {
    // os << "B" << (!sign(obj)?"~":"");
    if (!obj.isFixed())      os << "?";
    else if (obj.isTrue())   os << "+";
    else                     os << "-";

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Lit& obj) {
    os << "L" << (!sign(obj)?"~":"") << var(obj);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Clause& obj) {
    os << "C(";
    for (int i = 0; i < obj.sz; i++) {
        os << (i?",":"");
        os << obj[i];
    }
    os << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const WatchElem& obj) {
    os << "W(";
    switch (obj.type()) {
        case 0: os << *obj.pt();   break;
        case 2: os << "?" << obj.d1();   break;
        default:os << toLit(obj.d2());   break;
    }
    os << ")";
    return os;
}

// --------------------------------------------------------------------------------

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const vec<T>& obj) {
    os << "{";
    for (int i = 0; i < obj.size(); i++) {
        os << (i?",":"");
        os << obj[i];
    }
    os << "}";
    return os;
}

template<typename T>
inline std::stringstream vec1(vec<T> obj) {
    std::stringstream ss;
    ss << "{";
    for (int i = 0; i < obj.size(); i++) {
        ss << (i?",":"");
        ss << obj[i];
    }
    ss << "}";
    return ss;    
}

template<typename T>
inline std::stringstream vec2(vec<vec<T>> obj) {
    std::stringstream ss;
    ss << "{";
    for (int i = 0; i < obj.size(); i++) {
        ss << (i?",":"");
        ss << "{";
        for (int j = 0; j < obj[i].size(); j++) {
            ss << (j?",":"");
            ss << obj[i][j];
        }
        ss << "}";
    }
    ss << "}";
    return ss;    
}

// --------------------------------------------------------------------------------

inline std::string dbg(const Clause& data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}

inline std::string dbg(const vec<int>& data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}

inline std::string dbg_v1_int8(vec<int8_t>& obj) {
    std::stringstream ss;
    ss << obj;
    return ss.str();
}

inline std::string dbg_v1_boolview(const vec<BoolView>& data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}

inline std::string dbg_v1_clause(vec<Clause*>& obj) {
    std::stringstream ss;
    ss << "{";
    for (int i = 0; i < obj.size(); i++) {
        ss << (i?",":"");
        ss << *obj[i];
    }
    ss << "}";
    return ss.str();
}

inline std::string dbg_v2_watchelem(vec<vec<WatchElem>>& obj) {
    return vec2(obj).str();
}

inline std::string dbg_v2_lit(vec<vec<Lit>>& obj) {
    return vec2(obj).str();
}


inline std::string dbg_boolview(const BoolView& obj) {
    std::stringstream ss;
    ss << obj;
    return ss.str();
}

// --------------------------------------------------------------------------------

inline void launchdebugchuffed() {
    vec<BoolView> bs;
    vec<int> vi;

    BoolView b = BoolView();

    dbg(vi);

    dbg_boolview(b);

    dbg_v1_boolview(bs);

    dbg_v2_watchelem(sat.watches);
    dbg_v2_lit(sat.trail);
    dbg_v1_clause(sat.clauses);
    dbg_v1_int8(sat.assigns);
}

#endif