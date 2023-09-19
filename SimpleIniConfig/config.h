#pragma once

#define NOMINMAX
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <filesystem>
#include <any>
#include "iniconfig.h"
#include "color.h"
#include <variant>
#include "kbinds.h"


#ifndef CONFIG_H
#define CONFIG_H

// Your header file content here...

using hash_t = uint32_t;
namespace shared::hash
{
	constexpr uint64_t BASIS = 0x811c9dc5;
	constexpr uint64_t PRIME = 0x1000193;

	/// <summary>
	/// Creates hash of text during compile-time
	/// </summary>
	/// <param name="txt">The text that is going to be hashed</param>
	/// <param name="value">The current hash value</param>
	/// <returns>Hashed text</returns>
	inline constexpr hash_t get_const(const char* txt, const hash_t value = BASIS) noexcept
	{
		/// Recursive hashing
		return (txt[0] == '\0') ? value :
			get_const(&txt[1], (value ^ hash_t(txt[0])) * PRIME);
	}

	/// <summary>
	/// Creates hash of text during run-time
	/// </summary>
	/// <param name="str">The text that is going to be hashed</param>
	/// <returns>Hashed text</returns>
	inline hash_t get(const char* txt)
	{
		hash_t ret = BASIS;

		hash_t length = hash_t(strlen(txt));
		for (auto i = 0u; i < length; ++i)
		{
			/// OR character and multiply it with fnv1a prime
			ret ^= txt[i];
			ret *= PRIME;
		}

		return ret;
	}
}

namespace config
{



	struct item_t
	{
		item_t() = default;
		item_t(const std::string name, const hash_t type, const std::any var, const std::string section)
			: m_name(name), m_type(type), m_var(var), m_section(section)
		{};

		~item_t() = default;

		/// <summary>
		/// Cast the inner variable to the desired type and returns it
		/// </summary>
		/// <returns>Casted variable</returns>
		template< typename t >
		t& get()
		{
			return *reinterpret_cast<t*>(std::any_cast<t>(&m_var));
		}

		/// <summary>
		/// Sets the inner variable to the desired value
		/// </summary>
		/// <param name="val">Value that the variable will be set to</param>
		template< typename t >
		void set(t val)
		{
			m_var.emplace<t>(val);
		}

		std::string m_name;
		std::string m_section;
		hash_t m_type;
		std::any m_var;
	};

	/// <summary>
	/// Returns the registered config items vector
	/// </summary>
	/// <returns>Registered config items</returns>
	std::vector<item_t>& get_items();

	/// <summary>
	/// Adds a new config item into the vector
	/// </summary>
	/// <param name="name">Identifier of the variable ( has to be unique )</param>
	/// <param name="type">Type of the varialble</param>
	/// <param name="def">Default value of the variable</param>
	/// <returns>Index of the item in the vector</returns>
	template< typename t >
	uint32_t add_item(const std::string name, const hash_t type, const t def, const std::string section_name)
	{

		for (size_t i = 0; i < get_items().size(); ++i) { // fix in case of multi init 
			if (get_items()[i].m_name == name && get_items()[i].m_type == type && get_items()[i].m_section == section_name) {
				// Item already exists, return its index
				return static_cast<uint32_t>(i);
			}
		}
		get_items().push_back(item_t(name, type, std::make_any<t>(def), section_name));
		return static_cast<uint32_t>(get_items().size()) - 1u;
	}

	/// <summary>
	/// Gets config item variable from vector index
	/// </summary>
	/// <param name="index">Index of the config item</param>
	/// <returns>Config item variable</returns>
	template< typename t >
	t& get(const uint32_t index)
	{
		return get_items().at(index).get<t>();
	}

	/// <summary>
	/// Gets config item from vector index
	/// </summary>
	/// <param name="index">Index of the config item</param>
	/// <returns>Config item</returns>
	item_t& get_item(const uint32_t index);
	int does_item_exist(const std::string& section, const std::string& key);

	/// <summary>
	/// Saves current config
	/// </summary>
	/// <param name="config">Name of the config</param>
	void save(const std::string_view config);

	/// <summary>
	/// Loads current config
	/// </summary>
	/// <param name="config">Name of the config</param>
	/// <returns>Was config loaded</returns>
	bool load_settings(const std::string_view config);

	/// <summary>
	/// Sets the name of the current config directory
	/// </summary>
	/// <param name="config">Name of config directory</param>
	void set_config_directory(const std::string name);

	/// <summary>
	/// Gets the current config directory
	/// </summary>
	/// <returns>Name of the config directory</returns>
	std::string get_config_directory();

	template< typename t, size_t s >
	std::vector<t> create_filled_vector(const t& fill)
	{
		auto vec = std::vector<t>(s);
		std::fill(vec.begin(), vec.begin() + s, fill);
		return vec;
	}
	inline shared::col_t create_col_vector(const shared::col_t& fill) {
		return shared::col_t(fill);
	}
	template<typename t>
	t& ref(const uint32_t index) {
		return get_items().at(index).get<t>();
	}

	std::string MD5HashToUpper(const std::string& input);

}


/// <summary>
/// Creates hash of text during compile-time
/// </summary>
/// <param name="str">The text that is going to be hashed</param>
/// <returns>Hashed text</returns>
#define CT_HASH( str ) \
       [ ]( ) { \
           constexpr hash_t ret = shared::hash::get_const( str ); \
           return ret; \
       }( )

/// <summary>
/// Creates hash of text during run-time
/// </summary>
/// <param name="str">The text that is going to be hashed</param>
/// <returns>Hashed text</returns>
#define HASH( str ) shared::hash::get( str )


#define ADD_CFG_ITEM(type, name, def, section) const uint32_t c_##section##_##name = config::add_item<type>((#name),CT_HASH(#type), def, (#section)); // returns index key 

#define ADD_CFG_ITEM_VEC(type, datatype, size, name, def, section) \
    const uint32_t name = config::add_item<std::vector<datatype>>((#name), CT_HASH(#type), config::create_filled_vector<datatype, size>(def), (#section));

#define ADD_CFG_ITEM_HASHED(type, name, def, section) const uint32_t name = config::add_item<type>(CT_HASH(#name),CT_HASH(#type), def, CT_HASH(#section)); // returns index key 

#define ADD_CFG_ITEM_VEC_HASHED(type, datatype, size, name, def, section) \
    const uint32_t name = config::add_item<std::vector<datatype>>(CT_HASH(#name), CT_HASH(#type), config::create_filled_vector<datatype, size>(def), CT_HASH(#section));
#endif // CONFIG_H
