#pragma once

#include <set>
#include <string>
#include <vector>
#include <memory>

namespace pcre_parser
{
	using matches = std::multiset<std::string>;

	class RegexParser
	{
	public:
		explicit RegexParser(const char *pattern);
		~RegexParser();
		// TODO: also test what if these two are defaulted right here
		RegexParser(RegexParser &&) noexcept;
		RegexParser &operator=(RegexParser &&) noexcept;

		std::string single_match(const std::string &source, int start_offset = 0) const;
		matches all_matches(const std::string &source, int start_offset = 0) const;
	private:

		struct Impl;
		std::unique_ptr<Impl> impl;

		int process_pcre_exec(const std::string &source, int start_offset, std::vector<int> &outputs) const;
	};
}
