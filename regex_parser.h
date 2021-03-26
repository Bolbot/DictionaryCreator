#ifndef __REGEX_PARSER_H__
#define __REGEX_PARSER_H__

#define USING_PCRE_FOR_REGEX_PARSING 1

#if __has_include(<pcre.h>) && USING_PCRE_FOR_REGEX_PARSING
//#define PCRE_STATIC
#include <pcre.h>
#define __PCRE_IS_AVALIABLE__
#endif


#include <set>
#include <string>
#include <vector>
#include <sstream>
#include <exception>

#include <iostream>		// FOR DEBUG

#ifndef __PCRE_IS_AVALIABLE__
static_assert(false, "PCRE_IN_NOT_AVALIABLE");
#endif

namespace pcre_parser
{
	using matches = std::multiset<std::string>;

	constexpr size_t default_output_vector_size = 27;
	constexpr int default_pcre_compile_options = 0;
	constexpr unsigned char * const default_pcre_compile_table_ptr = nullptr;
#ifdef __PCRE_IS_AVALIABLE__
	constexpr pcre_extra * const default_pcre_exec_extra = nullptr;
#endif

	class RegexParser
	{
	public:
		explicit RegexParser(const char *pattern);
		std::string single_match(const std::string &source, int start_offset = 0) const;
		matches all_matches(const std::string &source, int start_offset = 0) const;
	private:
		int options;
		const char *error{ nullptr };
		int error_offset{ 0 };
		const unsigned char *table_pointer{ default_pcre_compile_table_ptr };
#ifdef __PCRE_IS_AVALIABLE__
		pcre *regex;
#endif

		int process_pcre_exec(const std::string &source, int start_offset, std::vector<int> &outputs) const;
	};
}

#endif
