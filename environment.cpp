#include <fstream>
#include <sstream>

#include "environment.hpp"

std::pair<std::vector<std::string>, bool> readlines(std::string filename) {
    std::ifstream f(filename);
    if(!f) {
        return { {}, false };
    }

    std::stringstream buffer;
    buffer << f.rdbuf();

    std::vector<std::string> lines;
    for(std::string line; std::getline(buffer, line);) {
        lines.push_back(line);
    }

    return { lines, true };
}
