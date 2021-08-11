#include "regex_parser.h"

#define USING_PCRE_FOR_REGEX_PARSING 1

#if __has_include(<pcre.h>) && USING_PCRE_FOR_REGEX_PARSING
	#include <pcre.h>
	#define __PCRE_IS_AVAILABLE__
#endif


#ifdef __PCRE_IS_AVAILABLE__

constexpr size_t default_output_vector_size = 27;
constexpr int default_pcre_compile_options = PCRE_UTF8 | PCRE_UCP;
constexpr int default_pcre_exec_options = 0;
constexpr unsigned char* const default_pcre_compile_table_ptr = nullptr;
#ifdef __PCRE_IS_AVAILABLE__
constexpr pcre_extra* const default_pcre_exec_extra = nullptr;
#endif

struct pcre_parser::RegexParser::Impl
{
	const char* error{ nullptr };
	int error_offset{ 0 };
	const unsigned char* table_pointer{ default_pcre_compile_table_ptr };
#ifdef __PCRE_IS_AVAILABLE__
	pcre* regex;
#endif

	Impl(const char *pattern);
};

pcre_parser::RegexParser::Impl::Impl(const char *pattern)
	:
	regex{ pcre_compile(pattern, default_pcre_compile_options, &error, &error_offset, table_pointer) }
{}

pcre_parser::RegexParser::RegexParser(const char *pattern)
	: impl{ new pcre_parser::RegexParser::Impl(pattern) }
{
	if (impl->regex == nullptr)
	{
		std::stringstream error_message;
		error_message << "Compilation of regular expression failed:\n\t" << pattern
			<< "\n" << std::string(impl->error_offset, '-')
			<< "\n(" << impl->error << ")";
			throw std::runtime_error(error_message.str());
	}
}

pcre_parser::RegexParser::~RegexParser() = default;

pcre_parser::RegexParser::RegexParser(RegexParser &&) noexcept = default;
pcre_parser::RegexParser &pcre_parser::RegexParser::operator=(pcre_parser::RegexParser &&) noexcept = default;

int pcre_parser::RegexParser::process_pcre_exec(const std::string &source, int start_offset, std::vector<int> &outputs) const
{
	int result = 0;
	do
	{
		result = pcre_exec(impl->regex, default_pcre_exec_extra,
			source.data(), static_cast<int>(source.size()),
			start_offset, default_pcre_exec_options, 
			outputs.data(), static_cast<int>(outputs.size()));

		if (result == 0)
		{
			outputs.resize(outputs.size() * 2);
		}
	}
	while (result == 0);

	if (result < 0 && result != PCRE_ERROR_NOMATCH)
	{
		std::string error_message{ "Matching of regular expression failed, error #" };
		error_message.append(std::to_string(result));
		throw std::runtime_error(error_message);
	}

	return result;
}

std::string pcre_parser::RegexParser::single_match(const std::string &source, int start_offset) const
{
	std::vector<int> outputs(default_output_vector_size);

	auto res = process_pcre_exec(source, start_offset, outputs);

	if (res == PCRE_ERROR_NOMATCH)
	{
		return {};
	}
	else
	{
		return source.substr(outputs[0], outputs[1] - outputs[0]);
	}
}

pcre_parser::matches pcre_parser::RegexParser::all_matches(const std::string &source, int start_offset) const
{
	matches every_match;
	std::vector<int> outputs(default_output_vector_size);

	int found = 0;
	while (found != PCRE_ERROR_NOMATCH)
	{
		if (outputs[0] < outputs[1])
		{
			every_match.emplace(source.substr(outputs[0], outputs[1] - outputs[0]));
		}

		start_offset = outputs[1];

		found = process_pcre_exec(source, start_offset, outputs);
	}

	return every_match;
}

#else

// comment following assert to allow a nonfunctional execution
static_assert(false, "PCRE IS NOT AVAILABLE");

pcre_parser::RegexParser::RegexParser(const char *pattern)
{}

std::string pcre_parser::RegexParser::single_match(const std::string &source, int start_offset) const
{
	return "PCRE is not available";
}

pcre_parser::matches pcre_parser::RegexParser::all_matches(const std::string &source, int start_offset) const
{
	return { "PCRE is not available" };
}

#endif
