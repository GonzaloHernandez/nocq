#ifndef debugstd_h
#define debugstd_h

#include <ostream>
#include <vector>
#include <sstream>

// --------------------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& os, const std::vector<int>& obj) {
    os << "{";
    for (int i = 0; i < obj.size(); i++) {
        os << obj[i];
        if (i<obj.size()-1) os << ",";
    }
    os << "}";
    return os;
}

// --------------------------------------------------------------------------------

inline std::string wsvi(std::vector<int>& data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}

// --------------------------------------------------------------------------------
inline void launchdebugstd() {
    std::vector<int> svi;
    wsvi(svi);
}

#endif