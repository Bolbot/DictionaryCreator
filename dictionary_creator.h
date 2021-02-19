#ifndef __DICTIONARY_CREATOR_H__
#define __DICTIONARY_CREATOR_H__

// Stupid windows.h is messing with min name, so following disables their dumb macros
#define NOMINMAX

#include <set>
#include <map>
#include <regex>
#include <vector>
#include <utility>
#include <fstream>
#include <algorithm>

#include "regex_parser.h"
#include "dictionary.h"
#include "dictionary_language.h"

namespace dictionary_creator
{
	class DictionaryCreator
	{
	//	const char *const name_lookbehind_pattern = u8R"((?<=[^.!?]\s)([A-Z][a-z]+))";		// just in case, for the reference
	//	std::regex word_pattern{ R"([[:alpha:]]{3,})" };					//
	//	std::regex name_pattern{ R"(([,[:alpha:]]+[[:space:]])+?([A-Z][a-z]+))" };		//
	//	std::regex name_at_the_start{ R"(^([A-Z][a-z]+){1}\s?.*$)" };				//

	public:
		DictionaryCreator(Language language);

		void add_input(std::ifstream &&input_stream);

		Dictionary parse_to_dictionary();

	private:
		Dictionary parse_one_file(std::ifstream &file_input);

		void remove_crlf(utf8_string &string) const;

		const Language language;
		std::vector<std::ifstream> input_files;

		utf8_string terminating_characters;
		pcre_parser::RegexParser proper_nouns_extractor;	// no shenanigans, just more detailed, perhaps a bit cumbersome code
		size_t minimal_substantial_word_length;
		std::regex name_at_the_start;
		std::regex word_pattern;
		std::regex name_pattern;
	};
}


#endif
