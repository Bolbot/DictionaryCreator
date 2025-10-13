#pragma once

#include "connections.h"
#include "dictionary_types.h"
#include "dictionary_language.h"

#ifndef BOOST_UNAVAILABLE
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#else
#include <set>
#include <map>
#endif // BOOST_UNAVAILABLE

#include <sstream>

namespace dictionary_creator
{
	std::string percent_encode(utf8_string string);

	definitions_t define_word(utf8_string word, Language language);
}
