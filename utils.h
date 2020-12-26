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
#include <exception>

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
	using JsonType = decltype(json::parse(input));

	JsonType result;
	bool result_valid = false;			// TODO: replace this with optional, maybe

	try
	{
		result = json::parse(input);
		result_valid = true;
	}
	catch (std::exception &e)
	{}

	return (result_valid ? result : JsonType{});
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

namespace lowercase_traits
{
	template <typename Char>
	struct lower_case_char_traits : public std::char_traits<Char>
	{
		static void assign(Char &dest, const Char &source)
		{
			if constexpr (std::is_same_v<Char, char>)
			{
				dest = std::tolower(source);
			}
			else
			{
				dest = std::towlower(source);
			}
		}

		static Char *move(Char *dest, const Char *source, size_t size)
		{
			for (size_t i = 0; i != size; ++i)
			{
				assign(dest[i], source[i]);
			}
			return dest;
		}

		static Char *copy(Char *dest, const Char *source, size_t size)
		{
			return move(dest, source, size);
		}

		static bool eq(Char a, Char b)
		{
			if constexpr (std::is_same_v<Char, char>)
			{
				return tolower(a) == tolower(b);
			}
			else
			{
				return towlower(a) == towlower(b);
			}
		}

		static bool lt(Char a, Char b)
		{
			if constexpr (std::is_same_v<Char, char>)
			{
				return tolower(a) < tolower(b);
			}
			else
			{
				return towlower(a) < towlower(b);
			}
		}

		static int compare(const Char *a, const Char *b, size_t n)
		{
			for (; n != 0; ++a, ++b, --n)
			{
				if (lt(*a, *b))
				{
					return -1;
				}
				else if (lt(*b, *a))
				{
					return 1;
				}
			}
			return 0;
		}

		static const Char *find(const Char *heystack, size_t size, Char needle)
		{
			for (size_t i = 0; i != size; ++i)
			{
				if (eq(heystack[i], needle))
				{
					return &heystack[i];
				}
			}
			return nullptr;
		}
	};

	template <typename Char, typename Traits, typename AnotherChar>
	std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &out, const std::basic_string<AnotherChar, Traits> &str)
	{
		return out.write(str.data(), str.size());
	}

	template <typename Char, typename Traits, typename AnotherTraits>
	std::basic_istream<Char, Traits> &getline(std::basic_istream<Char, Traits> &stream, std::basic_string<Char, AnotherTraits> &string, Char delimiter = '\n')
	{
		string.clear();

		Char current_char;
		while (stream.get(current_char) && current_char != delimiter)
		{
			string.push_back(current_char);
		}

		return stream;
	}

	template <typename Char, typename Traits, typename AnotherTraits>
	std::basic_string<Char, Traits> operator+(std::basic_string<Char, Traits> left, const std::basic_string<Char, AnotherTraits> &right)
	{
		for (Char c: right)
		{
			left.push_back(c);
		}
		return left;
	}
}

using WordType = std::basic_string<char, lowercase_traits::lower_case_char_traits<char>>;

#endif
