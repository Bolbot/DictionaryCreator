#ifndef __DICTIONARY_DEFINER_H__
#define __DICTIONARY_DEFINER_H__

#include "connections.h"
#include "dictionary_types.h"
#include "dictionary_language.h"

#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>

#include <sstream>

namespace dictionary_creator
{
	std::string percent_encode(utf8_string string);

	definitions_t define_word(utf8_string word, Language language);
}

#endif
