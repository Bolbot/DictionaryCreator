#pragma once

#ifndef BOOST_UNAVAILABLE
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#else
#include <string>
#include <map>
#include <set>
#endif // BOOST_UNAVAILABLE

#include <functional>
#include <exception>
#include <stdexcept>

namespace dictionary_creator
{
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
