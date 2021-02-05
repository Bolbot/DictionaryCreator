#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef _WIN32
#define __THIS_IS_WINDOWS__
#elif defined __unix__
#define __THIS_IS_UNIX__
#endif

#include <locale>
#include <string>
#include <fstream>
#include <codecvt>
#include <cwctype>
#include <iostream>
#include <exception>

#if __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
#define __NLOHMANN_JSON_IS_AVALIABLE__
#endif

#define __STRINGTYPE_IS_WSTRING__ 0

#if defined __STRINGTYPE_IS_WSTRING__ && __STRINGTYPE_IS_WSTRING__
using StringType = std::wstring;
static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter_to_wstring;

extern std::wostream &output;
extern std::wistream &input;

extern std::wostream &error_output;

using FileInputStream = std::basic_ifstream<wchar_t>;
using FileOutputStream = std::basic_ofstream<wchar_t>;
using InputStream = std::basic_istream<wchar_t>;
using OutputStream = std::basic_ostream<wchar_t>;
#else
using StringType = std::string;

extern std::ostream &output;
extern std::istream &input;
extern std::ostream &error_output;

using FileInputStream = std::basic_ifstream<char>;
using FileOutputStream = std::basic_ofstream<char>;
using InputStream = std::basic_istream<char>;
using OutputStream = std::basic_ostream<char>;
#endif

static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter_from_wstring;

#ifdef __NLOHMANN_JSON_IS_AVALIABLE__

template <typename InputType>
auto parse_json(InputType input)
{
	using JsonType	//= decltype(nlohmann::json::parse(input));
					= nlohmann::basic_json<std::map, std::vector, std::string>;

	JsonType result;
	bool result_valid = false;			// TODO: replace this with optional, maybe

	try
	{
		result = nlohmann::json::parse(input);
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
	return "NLOHMANN json feature is not avaliable";
}
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

template <typename Char = char>
using WordType = std::basic_string<Char, lowercase_traits::lower_case_char_traits<Char>>;

char lowercase_letter(char c);
wchar_t lowercase_letter(wchar_t w);

#endif
