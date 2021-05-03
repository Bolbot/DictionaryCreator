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
#include <memory>

#include "regex_parser.h"
#include "dictionary.h"
#include "dictionary_language.h"

namespace dictionary_creator
{
	class DictionaryCreator
	{
	public:
		DictionaryCreator(Language language);

		void add_input(std::unique_ptr<std::istream> &&uptr_to_stream);

		Dictionary parse_to_dictionary();

		Dictionary parse_line(utf8_string line) const;

	private:
		Dictionary parse_one_file(std::istream &file_input);

		void remove_crlf(utf8_string &string) const;

		Language language;
		std::vector<std::unique_ptr<std::istream>> input_files;

		size_t minimal_substantial_word_length;
		utf8_string terminating_characters;
		pcre_parser::RegexParser proper_nouns_extractor;
		pcre_parser::RegexParser linestarting_name_extractor;
		pcre_parser::RegexParser words_extractor;
	};
}


#endif
