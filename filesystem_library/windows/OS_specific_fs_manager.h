#pragma once

#include <string>

OPTIONAL_DECLSPEC extern const wchar_t forbidden_file_path_characters[7];

std::wstring read_user_input_line();

bool regular_file(const std::wstring &path) noexcept;

void fix_missing_extension(std::wstring &path);
