#include "fs_manager.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

bool UTF8_aware_console = true;

constexpr char forbidden_file_path_characters[2] = "\0";

std::string read_user_input_line()
{
	std::string result;
	while (result.empty())
	{
		std::cout << "> ";
		std::getline(std::cin, result);
	}
	return result;
}

bool regular_file(const std::string &path)
{
	struct stat statistics;

	if (lstat(path.c_str(), &statistics) == -1)
	{
		throw std::runtime_error("Failed to acquire file statistics");
	}

	return S_ISREG(statistics.st_mode);
}

void fix_missing_extension(std::string &path)
{
	auto last_period = path.rfind('.');
	auto last_slash = path.rfind('/');

	if (last_period == std::string::npos ||
		(last_slash != std::string::npos && last_period < last_slash))
	{
		path.append(".txt");
	}
}

const bool cin_stdio_sync = [] { std::cin.sync_with_stdio(false); return false; }();
