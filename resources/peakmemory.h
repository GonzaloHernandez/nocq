#include <fstream>
#include <string>
#include <sstream>

inline double getPeakMemoryMB() {
    std::ifstream status_file("/proc/self/status");
    std::string line;
    while (std::getline(status_file, line)) {
        if (line.rfind("VmHWM:", 0) == 0) {
            std::istringstream iss(line);
            std::string label, value_str, unit;
            iss >> label >> value_str >> unit;
            long value_kb = std::stol(value_str);
            return value_kb / 1024.0; // convert to MB
        }
    }
    return 0.0;
}