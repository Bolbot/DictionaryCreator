#ifndef __CONSOLE_FILE_SYSTEM_H__
#define __CONSOLE_FILE_SYSTEM_H__

#if __has_include(<filesystem>)
#include <filesystem>
#else
static_assert(false, "<filesystem> is not avaliable, make sure it's C++17");
#endif

#include <codecvt>
#include <string>
#include <set>

#include "utils.h"

namespace fs
{
	using namespace std::filesystem;
	using FSStringType = path::string_type;
	using fsCharType = path::value_type;

	extern FSStringType directory;
	extern std::set<FSStringType> text_files_extensions;

#ifdef __THIS_IS_WINDOWS__
	const FSStringType slash_or_backslash = LR"(\/)";
#else
	const FSStringType slash_or_backslash = R"(\/)";
#endif

	FSStringType to_fsstring(FSStringType fs) noexcept;
#ifdef __THIS_IS_WINDOWS__
	FSStringType to_fsstring(std::string str) noexcept;
#endif

	std::string stdstring(const std::string &s) noexcept;
#ifdef __THIS_IS_WINDOWS__
	std::string stdstring(const std::wstring &ws) noexcept;
#endif

	void request_the_extensions(FSStringType source);

	std::set<FSStringType> get_possible_extensions();

	std::vector<FileInputStream> provide_all_files_streams();

#if __STRINGTYPE_IS_WSTRING__
	std::wofstream output_to_file(FSStringType file_name);
#else
	std::ofstream output_to_file(FSStringType file_name);
#endif
	
	FSStringType generate_sorted_file_name(FSStringType prefix, size_t comparation_type);
}

#endif
