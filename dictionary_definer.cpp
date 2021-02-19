#include "dictionary_definer.h"

dictionary_creator::definitions_t dictionary_creator::define_word(utf8_string word, Language language)
{
	utf8_string api_request{ u8"https://api.dictionaryapi.dev/api/v2/entries/" };
	api_request += language_codes[static_cast<size_t>(language)];
	api_request += u8"/";
	api_request += word;

	utf8_string response = connections::get(api_request.c_str());
	auto json_t = parse_json(response);
	dictionary_creator::definitions_t map = json_to_definitions_map(json_t);

	return map;
}
