#ifndef DICTIONARY_DEFINER_IMPL_H
#define DICTIONARY_DEFINER_IMPL_H

// following heder defines nlohmann_json related templates
// it is intended for inner use in dictionary_definer and testing of this library
// it must be included after the dictionary_definer.h header since later includes proper container headers

#include <nlohmann/json.hpp>

template <typename InputType>
auto parse_json(InputType input) noexcept
{
	try
	{
		return nlohmann::json::parse(input);
	}
	catch ([[ maybe_unused ]] std::exception &not_trying_to_recover_return_empty_object)
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

#endif
