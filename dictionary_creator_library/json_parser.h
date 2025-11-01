#pragma once

#include "dictionary_types.h"

std::set<dictionary_creator::utf8_string> parse_json_to_definitions_set(const char* const json);

dictionary_creator::definitions_t parse_json_to_definitions_map(const char* const json);
