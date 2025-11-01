#include "dictionary_definer.h"

#include "connections.h"
#include "json_parser.h"

#include <sstream>

std::string dictionary_creator::percent_encode(dictionary_creator::utf8_string string)
{
	std::stringstream encoded;
	encoded << std::hex << std::uppercase << std::noshowbase;

	for (auto i: string)
	{
		if (i & 0x80)
		{
			encoded << '%' << (static_cast<uint16_t>(i) & 0xFF);
		}
		else
		{
			encoded << i;
		}
	}

	return encoded.str();
}

dictionary_creator::definitions_t dictionary_creator::define_word(utf8_string word, Language language)
{
	const utf8_string api_request_base{ u8"https://api.dictionaryapi.dev/api/v2/entries/" };
	if (language == dictionary_creator::Language::Uninitialized)
	{
		throw dictionary_runtime_error("Attempt to work with uninitalized language variable");
	}

	const auto IRI_encoded_word = percent_encode(word);

	const utf8_string api_request = api_request_base + language_codes[static_cast<size_t>(language)] + u8"/" + IRI_encoded_word;

	utf8_string response = connections::get(api_request.c_str());
	dictionary_creator::definitions_t map = parse_json_to_definitions_map(response.c_str());

	return map;
}
