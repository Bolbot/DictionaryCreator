#ifndef FILESYSTEM_FEATURES_H
#define FILESYSTEM_FEATURES_H

#include "config.h"
#if WINDOWS_H_IS_AVAILABLE && IO_H_IS_AVAILABLE && __has_include(<Windows.h>) && __has_include(<io.h>)
#include "platform_Windows.h"
#else
#include "platform_UNIX.h"
#endif

#include <fstream>

const bool cin_stdio_sync = [] { std::cin.sync_with_stdio(false); return false; }();

template <typename CharType>
void remove_enclosing_quotes(std::basic_string<CharType> &string) noexcept
{
	if (string.size() > 1 && string.front() == '\"' && string.back() == '\"')
	{
		string.erase(0, 1);
		string.pop_back();
	}
}

template <typename CharType>
bool valid_file_path(const std::basic_string<CharType> &string) noexcept
{
	for (auto i: forbidden_file_path_characters)
	{
		if (string.find(i) != std::string::npos)
		{
			return false;
		}
	}
	return true;
}

template <typename CharType>
bool file_already_exists(const std::basic_string<CharType> &path)
{
	std::ifstream attempt;
	attempt.exceptions(std::ios::failbit | std::ios::badbit);

	try
	{
		attempt.open(path);
		return true;
	}
	catch (std::ifstream::failure &doesnt_exist)
	{
		return false;
	}
}

#endif
