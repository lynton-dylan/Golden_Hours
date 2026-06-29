#pragma once
#include <string>
#include <fstream>
#include <sstream>

inline std::string load_html_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "Could not find " + filename;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}