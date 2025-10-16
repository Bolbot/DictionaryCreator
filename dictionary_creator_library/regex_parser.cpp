#include "regex_parser.h"

#if __has_include(<pcre2.h>) && PCRE2_CODE_UNIT_WIDTH == 8
	#include <pcre2.h>
	#define PCRE_IS_AVAILABLE
#endif

#ifdef PCRE_IS_AVAILABLE
constexpr size_t default_output_vector_size = 27;
constexpr int default_pcre_compile_options = PCRE2_UTF | PCRE2_UCP;
constexpr int default_pcre_exec_options = 0;
////constexpr unsigned char* const default_pcre_compile_table_ptr = nullptr;
//constexpr pcre_extra* const default_pcre_exec_extra = nullptr;

struct pcre_parser::RegexParser::Impl
{
	int error{ 0 };
	PCRE2_SIZE error_offset{ 0 };
	////const unsigned char* table_pointer{ default_pcre_compile_table_ptr };
	pcre2_code* regex;

	Impl(const char *pattern);
};

pcre_parser::RegexParser::Impl::Impl(const char *pattern)
	:
	regex{ pcre2_compile(reinterpret_cast<PCRE2_SPTR8>(pattern), PCRE2_ZERO_TERMINATED, default_pcre_compile_options, &error, &error_offset, /*table_pointer*/ nullptr)}
{}
#else
struct pcre_parser::RegexParser::Impl
{
	Impl(const char*) {}
	const char* error{ nullptr };
	int error_offset{ 0 };
	void* regex{ nullptr };
};
#endif // PCRE_IS_AVAILABLE


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

#ifdef PCRE_IS_AVAILABLE

int pcre_parser::RegexParser::process_pcre_exec(const std::string &source, int start_offset, std::vector<int> &outputs) const
{
	pcre2_match_data* match_data = pcre2_match_data_create_from_pattern_8(impl->regex, nullptr);
	int result = pcre2_match(impl->regex,
		reinterpret_cast<PCRE2_SPTR8>(source.data()), source.size(),
		start_offset, default_pcre_exec_options,
		match_data, nullptr);

	PCRE2_SIZE* raw_outputs = pcre2_get_ovector_pointer(match_data);
	if (!raw_outputs || (result <= 0 && result != PCRE2_ERROR_NOMATCH))
		throw std::runtime_error("Matching of regular expression failed");

	const auto match_number = pcre2_get_ovector_count(match_data);
	outputs.resize(match_number);
	for (size_t i = 0; i != match_number; ++i)
		outputs[i] = static_cast<int>(raw_outputs[i]);

	return result;
}

std::string pcre_parser::RegexParser::single_match(const std::string &source, int start_offset) const
{
	std::vector<int> outputs(default_output_vector_size);

	auto res = process_pcre_exec(source, start_offset, outputs);

	if (res == PCRE2_ERROR_NOMATCH)
		return {};
	else
		return source.substr(outputs[0], outputs[1] - outputs[0]);
}

pcre_parser::matches pcre_parser::RegexParser::all_matches(const std::string &source, int start_offset) const
{
	matches every_match;
	std::vector<int> outputs(default_output_vector_size, 0);

	int found = 0;
	while (found != PCRE2_ERROR_NOMATCH)
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

// uncomment following assert to disallow a nonfunctional execution
//static_assert(false, "PCRE IS NOT AVAILABLE");

std::string pcre_parser::RegexParser::single_match(const std::string &, int) const
{
	return "PCRE is not available";
}

pcre_parser::matches pcre_parser::RegexParser::all_matches(const std::string &, int) const
{
	return { "PCRE is not available" };
}

#endif // PCRE_IS_AVAILABLE
