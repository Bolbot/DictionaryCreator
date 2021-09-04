#ifndef __DICTIONARY_TYPES_H__
#define __DICTIONARY_TYPES_H__

#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <functional>
#include <exception>

#ifdef _WIN32
	#include <stdexcept>
#endif

namespace dictionary_creator
{
	using language_code_t = std::string;		// TODO: check if it's used and get rid of it if it's not

	using utf8_string = std::basic_string<char>;
	using definitions_t = std::map<utf8_string, std::set<utf8_string>>;
	using definer_t = std::function<definitions_t(utf8_string)>;

	using string_comp = std::function<bool(const utf8_string &, const utf8_string &)>;

	using letter_type = utf8_string;

	class dictionary_runtime_error : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};
}

#endif
