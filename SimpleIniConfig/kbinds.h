#pragma once
//#include "Windows.h"
#include "iniconfig.h"
#include "config.h"

struct MenuAndKeyData {
	std::map<std::string, std::map<int, std::string>> menuData;
	std::map<std::string, int> keyNames;
};

extern MenuAndKeyData gMenuKeyData;

MenuAndKeyData LoadMenuAndKeyNames(const std::string& filename);

class AmiKeyBind {
public:
	explicit AmiKeyBind(int defaultKey = 0) : keyCode(defaultKey), recordNextKeyPress(false) {
		keyName = GetKeyName(keyCode);
	}


	void startRecording() {
		recordNextKeyPress = true;
	}

	void recordKeyPress() {
		if (recordNextKeyPress) {
			for (int i = 0; i < 256; ++i) {
				if (GetAsyncKeyState(i) & 0x8000) {
					keyCode = i;
					keyName = GetKeyName(keyCode);
					recordNextKeyPress = false;
					break;
				}
			}
		}
	}

	bool isRecording() const {
		return recordNextKeyPress;
	}
	void setToPressedKey() {
		for (int i = 0; i < 256; ++i) {
			if (GetAsyncKeyState(i) & 0x8000) {
				keyCode = i;
				keyName = GetKeyName(keyCode);
				break;
			}
		}
	}

	bool isDown() const {
		return GetAsyncKeyState(keyCode) & 0x8000;
	}

	const std::string& toString() {
		keyName = GetKeyName(keyCode);
		return keyName;
	}

	void Set(int key) {
		keyCode = key;
	}

	int Get() {
		return keyCode;
	}


private:
	int keyCode;
	std::string keyName;
	bool recordNextKeyPress;
	std::string GetKeyName(int vk_code) const {
		switch (vk_code) {
		case VK_MBUTTON: return "Middle Mouse Button";
		case VK_LBUTTON: return "Left Mouse Button";
		case VK_RBUTTON: return "Right Mouse Button";
		case VK_CONTROL: return "Control Key";
		case VK_MENU: return "Alt Key";
		case VK_SHIFT: return "Shift Key";
		case VK_CAPITAL: return "Caps Lock";
		case VK_NUMLOCK: return "Num Lock";
		case VK_SCROLL: return "Scroll Lock";
		default: break;
		}
		UINT scancode = MapVirtualKey(vk_code, MAPVK_VK_TO_VSC);
		char key_name[50];
		int result = GetKeyNameTextA(scancode << 16, key_name, sizeof(key_name));
		if (result > 0) {
			return key_name;
		}
		else {
			return "Unknown Key: " + std::to_string(vk_code);
		}
	}

};

class KeyBindManager {
public:
	void addKeyBind(AmiKeyBind* keyBind) {
		keyBinds.push_back(keyBind);
	}

	void recordKeyPresses() {
		for (AmiKeyBind* keyBind : keyBinds) {
			keyBind->recordKeyPress();
		}
	}

	AmiKeyBind* getKeyBind(int index) {
		return keyBinds[index];
	}

private:
	std::vector<AmiKeyBind*> keyBinds;
};

extern KeyBindManager keyBindManager;
extern AmiKeyBind aimKeyBind;
extern AmiKeyBind triggerKeyBind;

struct Command {
	enum class ActionType {
		OnPress, OnRelease, Invalid
	};
	ActionType action;
	std::string sectionName;
	std::string valueName;
	std::string value;

	static Command parse(const std::string& commandString) {
		Command cmd;
		cmd.action = ActionType::Invalid;
		if (commandString.find("OnPress:") == 0) {
			cmd.action = ActionType::OnPress;
		}
		else if (commandString.find("OnRelease:") == 0) {
			cmd.action = ActionType::OnRelease;
		}
		size_t pos = commandString.find('.');
		if (pos == std::string::npos) return cmd;
		cmd.sectionName = commandString.substr(cmd.action == ActionType::OnPress ? 8 : 10, pos - (cmd.action == ActionType::OnPress ? 8 : 10));
		size_t equalPos = commandString.find('=', pos);
		if (equalPos == std::string::npos) return cmd;
		cmd.valueName = commandString.substr(pos + 1, equalPos - pos - 1);
		cmd.value = commandString.substr(equalPos + 1);
		return cmd;
	}
};

class FkeyBinds {
private:
	std::map<std::string, std::string> keybinds;  // To store key-value pairs
	int hotkey;  // To store the hotkey
	std::map<std::string, bool> previousKeyStates; // Tracks the previous states of keys to detect press/release

	int getKeyCodeFromKeyName(const std::string& keyName) {
		for (const auto& pair : gMenuKeyData.keyNames) {
			if (pair.first == keyName) {
				return pair.second;
			}
		}
		return -1; 
	}
public:
	FkeyBinds();
	FkeyBinds(const std::string& filepath);
	bool loadKeyBinds(const std::string& filepath);
	std::string getBind(const std::string& key);  // Returns the command for a specific key
	int getHotkey() const;  // Returns the hotkey
	std::string getActionForBind(const std::string& key, const std::string& actionType);  // Returns the command's action for a specific key based on action type
	void executeBind(const std::string& key, Command::ActionType actionType); // execute commands

	void continuousKeyCheck() {
		while (true) {
			if (hotkey != 1 && !GetAsyncKeyState(hotkey)) {
				continue;
			}
			for (const auto& bindPair : keybinds) {
				std::string keyName = bindPair.first;
				int keyCode = getKeyCodeFromKeyName(keyName);

				short keyState = GetAsyncKeyState(keyCode);
				bool isKeyPressed = keyState & 0x8000;

				if (isKeyPressed && (previousKeyStates.find(keyName) == previousKeyStates.end() || !previousKeyStates[keyName])) {
					executeBind(keyName, Command::ActionType::OnPress);
				}
				else if (!isKeyPressed && previousKeyStates[keyName]) {
					executeBind(keyName, Command::ActionType::OnRelease);
				}

				previousKeyStates[keyName] = isKeyPressed;
			}
			Sleep(10);
		}
	}
};

