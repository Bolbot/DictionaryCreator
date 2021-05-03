#ifndef PLATFORM_WINDOWS_H
#define PLATFORM_WINDOWS_H

#include <io.h>
#include <Windows.h>
#include <fcntl.h>
#include <string>
#include <iostream>

bool UTF8_awareness_for_windows_console = [] () { return SetConsoleOutputCP(CP_UTF8) && SetConsoleCP(CP_UTF8); }();

constexpr wchar_t forbidden_file_path_characters[] = L"<>\"?*|";

std::wstring read_user_input_line()
{
	HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);
	if (input_handle == nullptr || input_handle == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error("Failed to acquire input handle in MS Windows");
	}

	std::wstring result;

	do
	{
		std::cout << "> ";

		result = std::wstring(8192, wchar_t{});

		DWORD read = 0;

		if (ReadConsole(input_handle, result.data(), result.size(), &read, nullptr) == 0)
		{
			throw std::runtime_error("Failed to read from MS Windows console");
		}

		result.resize(read);

		while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == '\0'))
		{
			result.pop_back();
		}
	}
	while (result.empty());

	return result;
}

/* let's... don't..? mhm?
bool file_already_exists(const std::wstring &encoded_filename)
{
	bool result = false;

	int file_descriptor = -1;
	auto wsopen_res = _wsopen_s(&file_descriptor, encoded_filename.data(), _O_CREAT | _O_EXCL | _O_U8TEXT, _SH_DENYNO, _S_IWRITE);
	if (wsopen_res == EEXIST)
	{
		result = true;
	}
	else if (wsopen_res != 0)
	{
		throw std::runtime_error("Failed to access file for reading");
	}

	if (_close(file_descriptor) != 0)
	{
		throw std::runtime_error("Failed to close file descriptor");
	}

	return result;
}
*/

bool regular_file(const std::wstring &path) noexcept
{
	if (auto last_period = path.rfind(L'.'); last_period != std::string::npos)
	{
		return path.substr(last_period) != L".lnk";
	}
	return true;
}

void fix_missing_extension(std::wstring &path)
{
	auto last_period = path.rfind(L'.');
	auto last_slash = path.find_last_of(L"\\/");

	if (last_period == std::string::npos || 
		(last_slash != std::string::npos && last_period < last_slash))
	{
		path.append(L".txt");
	}
}

#endif
