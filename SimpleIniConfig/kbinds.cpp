#include "kbinds.h"
AmiKeyBind aimKeyBind(VK_MBUTTON);
AmiKeyBind triggerKeyBind(VK_RBUTTON);
KeyBindManager keyBindManager;
MenuAndKeyData gMenuKeyData;

MenuAndKeyData LoadMenuAndKeyNames(const std::string& filename) {
    MenuAndKeyData result;
    ini::INIParser parser;
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
#ifdef DEBUGLOG
        std::ofstream logFile;
        logFile.open("loadlog.txt", std::ios_base::app);
        logFile << "Error: Unable to open file : " << filename << std::endl;
        logFile.close();
#endif
        return result;  // Return empty result
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    parser.parse(buffer.str(), true);
    inFile.close();

    for (const auto& section : parser.sections) {
        const auto& sectionName = section.first;
        const auto& keyValueMap = section.second;

        if (sectionName == "KeyNames") {
            // Handle the KeyNames section differently
            for (const auto& kv : keyValueMap) {
                result.keyNames[kv.first] = std::stoi(kv.second);
            }
        }
        else {
            // For other sections, create a map of menu items indexed by integer
            std::map<int, std::string> menuItems;
            for (const auto& kv : keyValueMap) {
                menuItems[std::stoi(kv.second)] = kv.first;
            }
            result.menuData[sectionName] = menuItems;
        }
    }
    return result;
}




FkeyBinds::FkeyBinds(const std::string& filepath) {
    loadKeyBinds(filepath);
}
FkeyBinds::FkeyBinds() {
    hotkey = -1; 
}


bool FkeyBinds::loadKeyBinds(const std::string& filepath) {

    if (!std::filesystem::is_regular_file(filepath)) {
#ifdef DEBUGLOG
        std::cerr << "Config path is not a regular file." << std::endl;
#endif
        return false;
    }

    if (!std::filesystem::exists(filepath)) {
#ifdef DEBUGLOG
        std::cerr << "Config file doesn't exist." << std::endl;
#endif
        return false;
    }

    ini::INIParser parser;
    std::ifstream config_file(filepath, std::ios::in);
    if (!config_file.is_open()) {
#ifdef DEBUGLOG
        std::cerr << "Error opening the config file." << std::endl;
#endif
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(config_file)),
        std::istreambuf_iterator<char>());

    parser.parse(content, false);
    // Fetching the "KeyBinds" section and populating the keybinds map
    auto& keyBindsSection = parser.sections["KeyBinds"];
    auto& hotKey = parser.sections["KeyBinder"]["HotKey"];

    // Searching for HotKey
    auto hotKeyMatch = std::find_if(gMenuKeyData.keyNames.begin(), gMenuKeyData.keyNames.end(),
        [&](const std::pair<std::string, int>& entry) {
            return entry.first == hotKey;
        });

    if (hotKeyMatch == gMenuKeyData.keyNames.end()) {
#ifdef DEBUGLOG
        std::cerr << "Invalid HotKey specified in the config file." << std::endl;
#endif
    }
    else {
        int hotKeyCode = hotKeyMatch->second;
        hotkey = hotKeyCode;
    }

    // Processing keyBindsSection
    for (const auto& [keyName, command] : keyBindsSection) {
        auto keyMatch = std::find_if(gMenuKeyData.keyNames.begin(), gMenuKeyData.keyNames.end(),
            [&](const std::pair<std::string, int>& entry) {
                return entry.first == keyName;
            });

        if (keyMatch == gMenuKeyData.keyNames.end()) {
#ifdef DEBUGLOG
            std::cerr << "Invalid key specified for bind: " << keyName << std::endl;
#endif
            continue;
        }

        if (command.empty()) {
            continue;
        }

        keybinds[keyName] = command;
    }

    return true;
}


std::string FkeyBinds::getBind(const std::string& key) {
    auto it = keybinds.find(key);
    if (it != keybinds.end()) {
        return it->second;
    }
    return "";  // Return an empty string if not found
}

int FkeyBinds::getHotkey() const {
    return hotkey;
}

std::string FkeyBinds::getActionForBind(const std::string& key, const std::string& actionType) {
    std::string bindCommand = getBind(key);

    std::stringstream cmdStream(bindCommand);
    std::string singleCommand;

    while (std::getline(cmdStream, singleCommand, '|')) {
        auto separator = singleCommand.find(':');
        if (separator == std::string::npos) {
            continue;
        }

        std::string action = singleCommand.substr(0, separator);
        std::string value = singleCommand.substr(separator + 1);

        if (action == actionType) {
            return value;
        }
    }
    return "";
}

int getKeyCodeFromKeyName(const std::string& keyName) {
    for (const auto& pair : gMenuKeyData.keyNames) {
        if (pair.first == keyName) {
            return pair.second;
        }
    }
    return -1; // or throw an exception
}


void FkeyBinds::executeBind(const std::string& key, Command::ActionType actionType) {
    auto bind = getBind(key);
    if (bind.empty()) return;

    std::stringstream ss(bind);
    std::string commandString;
    while (std::getline(ss, commandString, '|')) {
        Command cmd = Command::parse(commandString);
        if (cmd.action != actionType) continue;

        int itemIndex = config::does_item_exist(cmd.sectionName, cmd.valueName);
        if (itemIndex < 0) {
#ifdef DEBUGLOG
            std::cerr << "Invalid item: " << cmd.sectionName << "." << cmd.valueName << std::endl;
#endif
            continue;
        }
        auto& item = config::get_item(itemIndex);

        switch (item.m_type) {
        case CT_HASH("bool"): {
            bool boolValue = cmd.value == "true" || cmd.value == "True";
            item.set<bool>(boolValue);
            break;
        }
        case CT_HASH("int"): {
            int intValue = std::stoi(cmd.value);
            item.set<int>(intValue);
            break;
        }
        case CT_HASH("float"): {
            float floatValue = std::stof(cmd.value);
            item.set<float>(floatValue);
            break;
        }
        case CT_HASH("shared::col_t"): {
            std::vector<std::string> colorComponents;
            std::stringstream colorStream(cmd.value);
            std::string component;
            while (std::getline(colorStream, component, ',')) {
                colorComponents.push_back(component);
            }

            if (colorComponents.size() == 4) { // Ensure we have the expected number of components
                shared::col_t colorValue(
                    std::stoi(colorComponents[0]),
                    std::stoi(colorComponents[1]),
                    std::stoi(colorComponents[2]),
                    std::stoi(colorComponents[3])
                );
                item.set<shared::col_t>(colorValue);
            }
            else {
#ifdef DEBUGLOG
                std::cerr << "Invalid color format for key " << key << ". Expected 4 components (R, G, B, A), found " << colorComponents.size() << "." << std::endl;
#endif
            }
            break;
        }
        case CT_HASH("std::string"): {
            item.set<std::string>(cmd.value);
            break;
        }
        case CT_HASH("std::vector<int>"): {
            std::vector<int> intValues;
            std::stringstream valueStream(cmd.value);
            std::string segment;
            while (std::getline(valueStream, segment, ',')) {
                intValues.push_back(std::stoi(segment));
            }
            item.set<std::vector<int>>(intValues);
            break;
        }
        case CT_HASH("std::vector<float>"): {
            std::vector<float> floatValues;
            std::stringstream valueStream(cmd.value);
            std::string segment;
            while (std::getline(valueStream, segment, ',')) {
                floatValues.push_back(std::stof(segment));
            }
            item.set<std::vector<float>>(floatValues);
            break;
        }
        default:
#ifdef DEBUGLOG
            std::cerr << "Unhandled item type: " << item.m_type << std::endl;
#endif
            break;
        }
    }
}

