#include "json_parser.h"
#include "json_parser_impl.h"

std::set<dictionary_creator::utf8_string> parse_json_to_definitions_set(const char* const json)
{
	const auto json_table = parse_json(json);

	return json_to_definitions_set(json_table);
}

dictionary_creator::definitions_t parse_json_to_definitions_map(const char* const json)
{
	const auto json_table = parse_json(json);

	return json_to_definitions_map(json_table);
}
