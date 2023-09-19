#include "config.h"
#include "MD5.h"
#include <type_traits> // issame
#define DEBUGLOG

namespace config
{
	std::string m_name = "cfg";
	std::vector<item_t>& get_items()
	{
		static std::vector<item_t> items;
		return items;
	}

	item_t& get_item(const uint32_t index)
	{
		return get_items().at(index);
	}

	int does_item_exist(const std::string& section, const std::string& key) {
		for (size_t i = 0; i < get_items().size(); i++) {
			auto caseInsensitiveCompare = [](const std::string& str1, const std::string& str2) {
				return std::equal(str1.begin(), str1.end(),
					str2.begin(), str2.end(),
					[](char c1, char c2) {
						return std::tolower(c1) == std::tolower(c2);
					});
			};

			if (caseInsensitiveCompare(get_items()[i].m_section, section) &&
				caseInsensitiveCompare(get_items()[i].m_name, key)) {
				return i;
			}
		}
		return -1; // Not found
	}



	void save(const std::string_view config)
	{
		std::filesystem::path config_path = std::filesystem::path(m_name) / config;
		std::filesystem::create_directory(config_path.parent_path());
		std::ofstream myfile;
		myfile.open("savelog.txt", std::ios_base::app);
		myfile << "filepath  " << config_path << std::endl;


		for (auto& item : get_items())
		{

			std::string section = item.m_section;
			std::string key = item.m_name;
			myfile << "item name  " << key << " item section  " << section << " item type  " << item.m_type << std::endl;
			switch (item.m_type)
			{
			case CT_HASH("bool"):
				ini::Write(section, key, item.get<bool>(), config_path.string());
				break;

			case CT_HASH("float"):
				ini::Write(section, key, item.get<float>(), config_path.string());
				break;

			case CT_HASH("int"):
				ini::Write(section, key, item.get<int>(), config_path.string());
				break;

			case CT_HASH("shared::col_t"):
			{
				ini::Write(section, key, item.get<shared::col_t>(), config_path.string());
				break;
			}
			case CT_HASH("std::string"):
				ini::Write_internal(section, key, item.get<std::string>(), config_path.string());
				break;

			case CT_HASH("std::vector<int>"):
				ini::Write(section, key, item.get<std::vector<int>>(), config_path.string());
				break;

			case CT_HASH("std::vector<float>"):
				ini::Write(section, key, item.get<std::vector<float>>(), config_path.string());
				break;
			case CT_HASH("AmiKeyBind"):
				ini::Write(section, key, item.get<AmiKeyBind>(), config_path.string());
				break;
			default:
				myfile << "Unknown item type for key: " << key << std::endl;
				break;
			}
		}


		myfile.close();
	}




	// This is a generic helper function to split a string into a vector
	template <typename T>
	std::vector<T> splitToVector(const std::string& s, char delimiter) {
		std::vector<T> elems;
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delimiter)) {
			if constexpr (std::is_same_v<T, int>) {
				elems.push_back(std::stoi(item));
			}
			else if constexpr (std::is_same_v<T, float>) {
				elems.push_back(std::stof(item));
			}
			else {
				elems.push_back(item);
			}
		}
		return elems;
	}

	bool load_settings(const std::string_view config) {
		std::filesystem::path config_path = std::filesystem::path(m_name) / config;

#ifdef DEBUGLOG
		std::ofstream logFile;
		logFile.open("loadlog.txt", std::ios_base::app);
		logFile << "Loading config from: " << config_path << std::endl;
#endif

		std::error_code err(0, std::generic_category());
		std::filesystem::create_directory(config_path.parent_path(), err);
		if (err.value() != 0) {
#ifdef DEBUGLOG
			logFile << "Error creating directory. Error code: " << err.value() << std::endl;
#endif
			return false;
		}

		if (!std::filesystem::is_regular_file(config_path)) {
#ifdef DEBUGLOG
			logFile << "Config path is not a regular file." << std::endl;
#endif
			return false;
		}

		if (!std::filesystem::exists(config_path)) {
#ifdef DEBUGLOG
			logFile << "Config file doesn't exist." << std::endl;
#endif
			return false;
		}

		ini::INIParser parser;
		std::ifstream config_file(config_path, std::ios::in);
		if (!config_file.is_open()) {
#ifdef DEBUGLOG
			logFile << "Error opening the config file." << std::endl;
#endif
			return false;
		}

		std::string content((std::istreambuf_iterator<char>(config_file)),
			std::istreambuf_iterator<char>());

		parser.parse(content, false);
#ifdef DEBUGLOG
		logFile << "file content " << content << std::endl;
#endif

		for (const auto& [sectionName, section] : parser.sections) {
			for (const auto& [key, value] : section) {
				auto item_index = does_item_exist(sectionName, key);
				if (item_index < 0) {
#ifdef DEBUGLOG
					logFile << "Key not found in items: " << key << std::endl;
#endif
					continue;
				}
				auto& cur_item = get_items().at(item_index);
#ifdef DEBUGLOG
				logFile << "Setting key: " << key << " with item type: " << cur_item.m_type << " and item index " << item_index << std::endl;
#endif

				switch (cur_item.m_type) {
				case CT_HASH("bool"):
					cur_item.set<bool>(value == "true");
#ifdef DEBUGLOG
					logFile << "Value set for key " << key << ": " << (value == "true") << std::endl;
#endif
					break;

				case CT_HASH("float"):
					cur_item.set<float>(std::stof(value));
#ifdef DEBUGLOG
					logFile << "Value set for key " << key << ": " << std::stof(value) << std::endl;
#endif
					break;

				case CT_HASH("int"):
					cur_item.set<int>(std::stoi(value));
#ifdef DEBUGLOG
					logFile << "Value set for key " << key << ": " << std::stoi(value) << std::endl;
#endif
					break;

				case CT_HASH("shared::col_t"):
				{
					std::stringstream ss(value);
					std::vector<std::string> values;
					std::string temp;
					while (getline(ss, temp, ',')) {
						values.push_back(temp);
					}

					auto colVal = shared::col_t(std::stoi(values[0]), std::stoi(values[1]), std::stoi(values[2]), std::stoi(values[3]));
					cur_item.set<shared::col_t>(colVal);
#ifdef DEBUGLOG
					logFile << "Value set for key " << key << ": RGB(" << std::stoi(values[0]) << ", " << std::stoi(values[1]) << ", " << std::stoi(values[2]) << ")" << std::endl;
#endif
					break;
				}

				case CT_HASH("std::string"):
					cur_item.set<std::string>(value);
#ifdef DEBUGLOG
					logFile << "Value set for key " << key << ": " << value << std::endl;
#endif
					break;

				case CT_HASH("std::vector<int>"):
				{
					std::vector<int> intVector = splitToVector<int>(value, ',');
					cur_item.set<std::vector<int>>(intVector);
#ifdef DEBUGLOG
					logFile << "Value set for key " << key << ": " << value << std::endl;
#endif
					break;
				}

				case CT_HASH("std::vector<float>"):
				{
					std::vector<float> floatVector = splitToVector<float>(value, ',');
					cur_item.set<std::vector<float>>(floatVector);
#ifdef DEBUGLOG
					logFile << "Value set for key " << key << ": " << value << std::endl;
#endif
					break;
				}

				case CT_HASH("AmiKeyBind"):
					cur_item.set<AmiKeyBind>(AmiKeyBind(std::stoi(value)));
#ifdef DEBUGLOG
					logFile << "Value set for key " << key << ": " << std::stoi(value) << std::endl;
#endif
					break;

				default:
#ifdef DEBUGLOG
					logFile << "Unknown type for key " << key << std::endl;
#endif
					break;
				}
			}
		}
#ifdef DEBUGLOG
		logFile.close();
#endif
		return true;
	}

	void set_config_directory(const std::string name) // relative or absolute path should be passed
	{
		m_name = name;
	}

	std::string get_config_directory()
	{
		return m_name;
	}


	std::string MD5HashToUpper(const std::string& input) {
		MD5 md5;
		std::string digest = md5.digestString((char*)input.c_str());
		std::transform(digest.begin(), digest.end(), digest.begin(), ::toupper);
		return digest;
	}
}

