#pragma once

#include "dictionary_types.h"
#include "dictionary_language.h"

namespace dictionary_creator
{
	std::string percent_encode(utf8_string string);		// used by dictionary_creator

	definitions_t define_word(utf8_string word, Language language);
}
