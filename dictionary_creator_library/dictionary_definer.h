#ifndef __DICTIONARY_DEFINER_H__
#define __DICTIONARY_DEFINER_H__

#include "connections.h"
#include "dictionary_types.h"
#include "dictionary_language.h"

#include <sstream>

#if __has_include(<nlohmann/json.hpp>)

#include <nlohmann/json.hpp>

template <typename InputType>
auto parse_json(InputType input) noexcept
{
	try
	{
		return nlohmann::json::parse(input);
	}
	catch (std::exception &e)
	{
		return nlohmann::basic_json<std::map, std::vector, std::string>{};
	}
}

template <typename J>
std::set<dictionary_creator::utf8_string> json_to_definitions_set(const J &json_table)
{
	std::set<dictionary_creator::utf8_string> definitions;

	if (json_table.contains(u8"definition"))
	{
		definitions.emplace(json_table[u8"definition"]);
	}

	for (const auto &level : json_table)
	{
		if (level.is_object() || level.is_array())
		{
			auto current = json_to_definitions_set(level);
			definitions.merge(current);
		}
	}

	return definitions;
}
	
template <typename J>
dictionary_creator::definitions_t json_to_definitions_map(const J &json_table)
{
	dictionary_creator::definitions_t map;

	if (json_table.contains(u8"partOfSpeech"))
	{
		dictionary_creator::utf8_string current_part_of_speech = json_table[u8"partOfSpeech"];

		auto current_definitions = json_to_definitions_set(json_table);
		map[current_part_of_speech] = std::move(current_definitions);
	}

	for (const auto &level : json_table)
	{
		if (level.is_object() || level.is_array())
		{
			auto current = json_to_definitions_map(level);
			map.merge(current);
		}
	}

	return map;
}

#else
template <typename InputType>
auto parse_json(InputType input)
{
	return "NLOHMANN json feature is not avaliable";
}
#endif

namespace dictionary_creator
{
	std::string percent_encode(utf8_string string);

	definitions_t define_word(utf8_string word, Language language);
}

#endif
