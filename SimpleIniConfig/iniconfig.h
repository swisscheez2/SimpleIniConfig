#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include "color.h"
// Forward declaration
class AmiKeyBind;



namespace ini {
    class INIParser {
    public:
        std::map<std::string, std::map<std::string, std::string>> sections;

        void parse(const std::string& content, bool reverseKeyValue = false) {
            std::stringstream ss(content);
            std::string line;
            std::string currentSection;

            while (std::getline(ss, line)) {
                line = trim(line);
                if (line.size() > 2 && line[0] == '[' && line[line.size() - 1] == ']') {
                    currentSection = line.substr(1, line.size() - 2);
                }
                else {
                    auto separator = line.find('=');
                    if (separator == std::string::npos) {
                        continue; // Invalid line, skip
                    }

                    std::string key, value;
                    if (reverseKeyValue) {
                        key = trim(line.substr(separator + 1));
                        value = trim(line.substr(0, separator));
                    }
                    else {
                        key = trim(line.substr(0, separator));
                        value = trim(line.substr(separator + 1));
                    }
                    sections[currentSection][key] = value;
                }
            }
        }
    private:
        std::string trim(const std::string& str) {
            auto start = str.find_first_not_of(" \t");
            auto end = str.find_last_not_of(" \t");
            if (start == std::string::npos || end == std::string::npos) {
                return "";
            }
            return str.substr(start, end - start + 1);
        }
    };

    void Write_internal(const std::string& section, const std::string& key, const std::string& value, const std::string& file);
    // Overloads for types
    //col
    void Write(const std::string& section, const std::string& key, const shared::col_t& color, const std::string& file);
    //keybind
    void Write(const std::string& section, const std::string& key, AmiKeyBind value, const std::string& file);
    // int
    void Write(const std::string& section, const std::string& key, int value, const std::string& file);
    // float
    void Write(const std::string& section, const std::string& key, float value, const std::string& file);
    // bool
    void Write(const std::string& section, const std::string& key, bool value, const std::string& file);
    // vector<int>
    void Write(const std::string& section, const std::string& key, const std::vector<int>& values, const std::string& file);
    // vector<float>
    void Write(const std::string& section, const std::string& key, const std::vector<float>& values, const std::string& file);
    // vector<bool>
    void Write(const std::string& section, const std::string& key, const std::vector<bool>& values, const std::string& file);
}

