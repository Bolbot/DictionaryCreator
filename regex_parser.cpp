#include "regex_parser.h"

#ifdef __PCRE_IS_AVALIABLE__

pcre_parser::RegexParser::RegexParser(const char *pattern)
	: options{ default_pcre_compile_options },
		 regex{ pcre_compile(pattern, options, &error, &error_offset, table_pointer) }
{
	if (regex == nullptr)
	{
		std::stringstream error_message;
		error_message << "Compilation of regular expression failed:\n\t" << pattern
			<< "\n" << std::string(error_offset, '-')
			<< "\n(" << error << ")";
			throw std::runtime_error(error_message.str());
	}
}

int pcre_parser::RegexParser::process_pcre_exec(const std::string &source, int start_offset, std::vector<int> &outputs) const
{
	int result = 0;
	do
	{
		result = pcre_exec(regex, default_pcre_exec_extra,
				source.data(), source.size(), start_offset,
				options, outputs.data(), outputs.size());

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
		return "No match found";
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

pcre_parser::RegexParser::RegexParser(const char *pattern)
{}

std::string pcre_parser::RegexParser::single_match(const std::string &source, int start_offset) const
{
	return "PCRE is not avaliable";
}

pcre_parser::matches pcre_parser::RegexParser::all_matches(const std::string &source, int start_offset) const
{
	return { "PCRE is not avaliable" };
}

#endif
