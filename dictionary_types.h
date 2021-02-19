#ifndef __DICTIONARY_TYPES_H__
#define __DICTIONARY_TYPES_H__

#include <string>
#include <set>
#include <map>
#include <functional>

namespace dictionary_creator
{
	using language_code_t = std::string;

	using utf8_string = std::basic_string<char>;
	using definitions_t = std::map<utf8_string, std::set<utf8_string>>;
	using definer_t = std::function<definitions_t(utf8_string)>;

	using letter_type = utf8_string;
}

#endif
