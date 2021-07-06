#ifndef FS_MANAGER_H
#define FS_MANAGER_H

// must be defined before including OS specific header as it uses it
#ifndef OPTIONAL_DECLSPEC
#define OPTIONAL_DECLSPEC
#endif

#include "OS_specific_fs_manager.h"

#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>


OPTIONAL_DECLSPEC extern bool UTF8_aware_console;

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
	catch ([[ maybe_unused ]] std::ifstream::failure &file_just_doesnt_exist)
	{
		return false;
	}
}

inline auto get_input_file_path()
{
	std::cout << "Specify input file:\n";

	while (true)
	{
		auto file_path = read_user_input_line();

		remove_enclosing_quotes(file_path);

		if (valid_file_path(file_path) == false)
		{
			std::cout << "Invalid file path. Try another.\n";
			continue;
		}

		if (file_already_exists(file_path) == false)
		{
			std::cout << "File does not exist. Try another.\n";
			continue;
		}

		if (regular_file(file_path) == false)
		{
			std::cout << "Not a file. Try another.\n";
			continue;
		}

		return file_path;
	}
}

inline auto get_output_file_path()
{
	std::cout << "Specify output file (.txt if no other extension specified):\n";

	while (true)
	{
		auto file_path = read_user_input_line();

		remove_enclosing_quotes(file_path);

		if (valid_file_path(file_path) == false)
		{
			std::cout << "Invalid file path. Try another.\n";
			continue;
		}

		fix_missing_extension(file_path);

		if (file_already_exists(file_path))
		{
			std::cout << "File already exists. Lose it's previous contents and write there anyway? ([Y]es / [n]o)\n> ";

			char choice = char{};
			std::cin >> choice;
			std::cin.ignore(std::cin.rdbuf()->in_avail(), '\n');

			if (choice != 'Y')
			{
				std::cout << "Specify another file.\n";
				continue;
			}
		}

		if (std::ofstream(file_path).good() && regular_file(file_path) == false)
		{
			std::cout << "Not a regular file. Try another.\n";
			continue;
		}

		return file_path;
	}
}


#endif
