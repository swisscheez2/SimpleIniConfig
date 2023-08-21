#include "iniconfig.h"

namespace ini {
    void Write_internal(const std::string& section, const std::string& key, const std::string& value, const std::string& file) {
        std::ifstream in(file);
        std::ostringstream temp;
        std::string line;
        bool sectionFound = false;
        bool keyWritten = false;
        // Read the entire file content and process it
        while (std::getline(in, line)) {
            if (line == "[" + section + "]") {
                sectionFound = true;
                temp << line << '\n';
                // Read and process the key-value pairs in the section
                while (std::getline(in, line) && !line.empty() && line[0] != '[') {
                    if (line.substr(0, line.find('=')) != key) {
                        temp << line << '\n';
                    }
                    else {
                        temp << key << '=' << value << '\n';
                        keyWritten = true;
                    }
                }

                if (!keyWritten) {
                    temp << key << '=' << value << '\n';
                }

                // If the section ends and the next section begins, add an extra newline
                if (!line.empty() && line[0] == '[') {
                    temp << '\n';
                }

            }
            temp << line << (in.eof() ? "" : "\n");
        }

        // If the section was not found, append it and the key-value to the end
        if (!sectionFound) {
            temp << (temp.str().empty() ? "" : "\n") << "[" << section << "]\n" << key << '=' << value << '\n';
        }

        in.close();

        std::ofstream out(file);
        out << temp.str();
        out.close();
    }

    std::string Read(const std::string& section, const std::string& key, const std::string& file) {
        std::ifstream in(file);
        std::string line;

        while (std::getline(in, line)) {
            if (line == "[" + section + "]") {
                while (std::getline(in, line) && !line.empty() && line[0] != '[') {
                    std::string currentKey = line.substr(0, line.find('='));
                    if (currentKey == key) {
                        return line.substr(line.find('=') + 1);
                    }
                }
            }
        }

        in.close();
        return "";  // Return empty string if key or section wasn't found
    }

    void Write(const std::string& section, const std::string& key, int value, const std::string& file) {
        Write_internal(section, key, std::to_string(value), file);
    }

    void Write(const std::string& section, const std::string& key, float value, const std::string& file) {
        Write_internal(section, key, std::to_string(value), file);
    }

    void Write(const std::string& section, const std::string& key, bool value, const std::string& file) {
        Write_internal(section, key, value ? "true" : "false", file);
    }

    void Write(const std::string& section, const std::string& key, const std::vector<int>& values, const std::string& file) {
        std::string concatenated = "";
        for (size_t i = 0; i < values.size(); ++i) {
            concatenated += std::to_string(values[i]);
            if (i != values.size() - 1) { // Check if it's not the last element
                concatenated += ",";
            }
        }
        Write_internal(section, key, concatenated, file);
    }

    void Write(const std::string& section, const std::string& key, const std::vector<float>& values, const std::string& file) {
        std::string concatenated = "";
        for (size_t i = 0; i < values.size(); ++i) {
            concatenated += std::to_string(values[i]);
            if (i != values.size() - 1) { // Check if it's not the last element
                concatenated += ",";
            }
        }
        Write_internal(section, key, concatenated, file);
    }
      
    void Write(const std::string& section, const std::string& key, const std::vector<bool>& values, const std::string& file) {
        std::string concatenated = "";
        for (bool val : values) {
            concatenated += val ? "true," : "false,";
        }
        Write_internal(section, key, concatenated, file);
    }
    // Read overloads
    int ReadInt(const std::string& section, const std::string& key, const std::string& file) {
        return std::stoi(Read(section, key, file));
    }

    float ReadFloat(const std::string& section, const std::string& key, const std::string& file) {
        return std::stof(Read(section, key, file));
    }

    bool ReadBool(const std::string& section, const std::string& key, const std::string& file) {
        std::string value = Read(section, key, file);
        return value == "true";
    }

    std::vector<int> ReadVectorInt(const std::string& section, const std::string& key, const std::string& file) {
        std::string value = Read(section, key, file);
        std::stringstream ss(value);
        std::string token;
        std::vector<int> values;
        while (std::getline(ss, token, ',')) {
            values.push_back(std::stoi(token));
        }
        return values;
    }

    std::vector<float> ReadVectorFloat(const std::string& section, const std::string& key, const std::string& file) {
        std::string value = Read(section, key, file);
        std::stringstream ss(value);
        std::string token;
        std::vector<float> values;
        while (std::getline(ss, token, ',')) {
            values.push_back(std::stof(token));
        }
        return values;
    }

    std::vector<bool> ReadVectorBool(const std::string& section, const std::string& key, const std::string& file) {
        std::string value = Read(section, key, file);
        std::stringstream ss(value);
        std::string token;
        std::vector<bool> values;
        while (std::getline(ss, token, ',')) {
            values.push_back(token == "true");
        }
        return values;
    }
}
