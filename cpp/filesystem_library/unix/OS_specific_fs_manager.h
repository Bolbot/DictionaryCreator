#pragma once

#include <string>

extern const char forbidden_file_path_characters[2];

std::string read_user_input_line();

bool regular_file(const std::string &path);

void fix_missing_extension(std::string &path);

extern const bool cin_stdio_sync;
