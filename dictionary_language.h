#ifndef __DICTIONARY_LANGUAGE_H__
#define __DICTIONARY_LANGUAGE_H__

#include <string>
#include <vector>

#include "dictionary_types.h"

// This part is supposed to handle language issues like following:
//
// 	-- whether a word can start with a capital letter (german nouns can)
// 	-- how the sorting is done (ä is near a, not after z)
// 	-- how to specify the language for the dictionary - enum class Language that also serves as index in respective arrays
// 	-- consider creating a base class that is language-aware and has constant public member Language

namespace dictionary_creator
{
	enum class Language : size_t
	{
		English = 1, French = 2, Russian = 3, German = 4
	};

	const std::vector<language_code_t> language_codes
	{
		u8"", u8"en", u8"fr", u8"ru", u8"de"
	};

	const std::vector<std::string> undefined_warnings
	{
		u8"", u8"Definition is absent", u8"La définition est absente", u8"Определение отсутствует", u8"Definition ist nicht vorhanden"
	};

	const std::vector<size_t> minimal_substantial_word_length
	{
		0, 3, 3, 3, 3
	};

	const std::vector<std::string> terminating_characters
	{
		u8"", u8".?!", u8".!?", u8".?!", u8".?!"
	};

	const std::vector<utf8_string> uppercase_letters
	{
		u8"", u8"A-Z", u8"A-ZÀÂÆÇÈÉÊËÏÎÔŒÙŸ", u8"А-Я", u8"A-ZÄÖÜ"
	};
	const std::vector<utf8_string> lowercase_letters
	{
		u8"", u8"a-z", u8"a-zàâéèêëïîôùûçœæ", u8"а-я", u8"a-zäöüß"
	};

	// Following are obsolete and are to be removed along with respective comments in DictionaryCreator constructor definition

	const std::vector<std::string> general_word
	{
		u8"", u8"[[:alpha:]]"	// obsolete
	};

	const std::vector<std::string> general_name	// obsolete
	{
		u8"", u8"([A-Z][a-z]+)", u8"([A-ZÀÂÆÇÈÉÊËÏÎÔŒÙŸ][a-zàâéèêëïîôùûçœæ]+)", u8"([А-Я][а-я]+)", "([A-ZÄÖÜ][a-zäöüß]+)"
	};

	const std::vector<std::string> nonterminating	// obsolete
	{
		u8"", u8R"(([,[:alpha:]]+[[:space:]]))", u8"", u8"", u8""
	};

	const std::vector<std::string> linestarting_names	// obsolete
	{
		u8"", u8R"(^([A-Z][a-z]+){1}\s?.*$)", u8"", u8"", u8""
	};
}

#endif
