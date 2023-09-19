#include "iniconfig.h"
#include "kbinds.h"
namespace ini {
    void Write_internal(const std::string& section, const std::string& key, const std::string& value, const std::string& file) {
        std::ifstream in(file);
        std::ostringstream temp;
        std::string line;
        bool sectionFound = false;
        bool keyWritten = false;
        while (std::getline(in, line)) {
            if (line == "[" + section + "]") {
                sectionFound = true;
                temp << line << '\n';
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

    void Write(const std::string& section, const std::string& key, int value, const std::string& file) {
        Write_internal(section, key, std::to_string(value), file);
    }

    void Write(const std::string& section, const std::string& key, AmiKeyBind value, const std::string& file) {
        Write_internal(section, key, std::to_string(value.Get()), file);
    }

    void Write(const std::string& section, const std::string& key, const shared::col_t& color, const std::string& file) {
        std::string concatenated = std::to_string(color.r()) + "," +
            std::to_string(color.g()) + "," +
            std::to_string(color.b()) + "," +
            std::to_string(color.a());
        Write_internal(section, key, concatenated, file);
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

}
