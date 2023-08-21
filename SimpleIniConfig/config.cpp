#include "config.h"

namespace config
{
	std::string m_name = "hashed_cfg";

	std::vector<item_t>& get_items()
	{
		static std::vector<item_t> items;
		return items;
	}

	item_t& get_item(const uint32_t index)
	{
		return get_items().at(index);
	}

	int does_item_exist(const std::string name)
	{
		/// Go through every item and check if name matches
		for (auto i = 0u; i < get_items().size(); i++)
			if (get_items().at(i).m_name == name)
				return i;

		return -1;
	}

	void save(const std::string_view config)
	{
		std::filesystem::path config_path = std::filesystem::path(m_name) / config;

		if (!config_path.is_absolute()) {
			// Ensure directory exists for relative paths
			std::filesystem::create_directory(config_path.parent_path());
		}

		for (auto& item : get_items())
		{
		
			std::string section = item.m_section;
			std::string key = item.m_name;

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

			case CT_HASH("<shared::col_t>"):
			{
				ini::Write(section, key, item.get<std::vector<int>>(), config_path.string());
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

			case CT_HASH("std::vector<bool>"):
				ini::Write(section, key, item.get<std::vector<bool>>(), config_path.string());
				break;

			default:
				break;
			}
		}
	}

	std::variant<bool, int, float, std::string, shared::col_t, std::vector<int>, std::vector<float>> inferType(const std::string& value) {
		// Check for boolean
		if (value == "true" || value == "false") {
			return value == "true";
		}

		// Check for number (int or float)
		else if (value.find_first_not_of(".0123456789") == std::string::npos) {
			if (value.find('.') != std::string::npos) {
				return std::stof(value);
			}
			else {
				return std::stoi(value);
			}
		}

		// Check for col_t (or other comma-separated types)
		else if (value.find(',') != std::string::npos) {
			std::stringstream ss(value);
			std::vector<std::string> values;
			std::string temp;
			while (getline(ss, temp, ',')) {
				values.push_back(temp);
			}

			if (values.size() == 4) { 
				return shared::col_t(
					std::stoi(values[0]),
					std::stoi(values[1]),
					std::stoi(values[2]),
					std::stoi(values[3])
				);
			}
			else {
				std::vector<int> intVector;
				for (const auto& val : values) {
					intVector.push_back(std::stoi(val));
				}
				return intVector;
			}
		}

		// Default to string
		else {
			return value;
		}
	}


	// Load function using INI format
	bool load(const std::string_view config) {
		std::filesystem::path config_path = std::filesystem::path(m_name) / config;

		if (!config_path.is_absolute()) {
			std::error_code err(0, std::generic_category());
			std::filesystem::create_directory(config_path.parent_path(), err);
			if (err.value() != 0)
				return false;
		}

		if (!std::filesystem::is_regular_file(config_path))
			return false;

		if (!std::filesystem::exists(config_path))
			return false;

		ini::INIParser parser;
		std::ifstream config_file(config_path, std::ios::in);
		std::string content((std::istreambuf_iterator<char>(config_file)),
		std::istreambuf_iterator<char>());
		parser.parse(content);

		for (const auto& [sectionName, section] : parser.sections) {
			for (const auto& [key, value] : section) {
				auto item_index = does_item_exist(key);
				if (item_index < 0) // item not in Items list
					continue;
			
				auto inferredValue = inferType(value);

				auto& cur_item = get_items().at(item_index);
				if (std::holds_alternative<bool>(inferredValue)) {
					cur_item.set<bool>(std::get<bool>(inferredValue));
				}
				else if (std::holds_alternative<int>(inferredValue)) {
					cur_item.set<int>(std::get<int>(inferredValue));
				}
				else if (std::holds_alternative<float>(inferredValue)) {
					cur_item.set<float>(std::get<float>(inferredValue));
				}
				else if (std::holds_alternative<std::string>(inferredValue)) {
					cur_item.set<std::string>(std::get<std::string>(inferredValue));
				}
				else if (std::holds_alternative<shared::col_t>(inferredValue)) {
					cur_item.set<shared::col_t>(std::get<shared::col_t>(inferredValue));
				}
				else if (std::holds_alternative<std::vector<int>>(inferredValue)) {
					cur_item.set<std::vector<int>>(std::get<std::vector<int>>(inferredValue));
				}
				else if (std::holds_alternative<std::vector<float>>(inferredValue)) {
					cur_item.set<std::vector<float>>(std::get<std::vector<float>>(inferredValue));
				}
			}
		}
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
}