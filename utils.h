#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef _WIN32
#define __THIS_IS_WINDOWS__
#elif defined __unix__
#define __THIS_IS_UNIX__
#endif

#include <filesystem>
#include <string>
#include <codecvt>
#include <iostream>

#if __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
#define __NLOHMANN_JSON_IS_AVALIABLE__
#endif

namespace fs = std::filesystem;
using FSStringType = fs::path::string_type;
using fsCharType = fs::path::value_type;
using StringType = std::string;
#ifdef __NLOHMANN_JSON_IS_AVALIABLE__
using json = nlohmann::json;
template <typename InputType>
auto parse_json(InputType input)
{
	return json::parse(input);
}
#else
template <typename InputType>
StringType parse_json(InputType input)
{
	return "NLOKNANN json feature is not avaliable";
}
#endif
extern std::ostream &output;
extern std::istream &input;
using FileInputStream = std::ifstream;
using FileOutputStream = std::ofstream;

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



#endif
