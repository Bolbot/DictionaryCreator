#include <iostream>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <fstream>
#include <filesystem>
#include <clocale>
#include <typeinfo>
#include <regex>
#include <algorithm>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#ifdef _WIN32
#define __THIS_IS_WINDOWS__
#elif defined __unix__
#define __THIS_IS_UNIX__
#endif

using json = nlohmann::json;
namespace fs = std::filesystem;
using CharType = fs::path::value_type;
using StringType = fs::path::string_type;
using StringStreamType = std::basic_stringstream<CharType>;

#ifdef __THIS_IS_WINDOWS__
std::basic_ostream<CharType> &output = std::wcout;
std::basic_istream<CharType> &input = std::wcin;
StringType input_terminator_string{ L"!" };
using FileOutputStream = typename std::wofstream;
using FileInputStream = typename std::wifstream;
StringType DefaultDictionaryName{ L"dictionary.txt" };
auto upper_case_letter = towupper;
#else
std::basic_ostream<CharType> &output = std::cout;
std::basic_istream<CharType> &input = std::cin;
StringType input_terminator_string{ "!" };
using FileOutputStream = typename std::ofstream;
using FileInputStream = typename std::ifstream;
StringType DefaultDictionaryName{ "dictionary.txt" };
auto upper_case_letter = toupper;
#endif

namespace [[ deprecated ]] lowercase_traits
{
	template <typename Char>
	struct lower_case_char_traits : public std::char_traits<Char>
	{
		static void assign(Char &dest, const Char &source)
		{
			if constexpr (std::is_same_v<CharType, char>)
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
	std::basic_ostream<Char, Traits> &operator<<(std::basic_ostream<Char, Traits> &out, const std::basic_string<AnotherChar> &str)
	{
		return out.write(str.data(), str.size());
	}
}

// Further TODO: implement the part of speech as
// (noun):	Definition of this word as a noun
// (adjective):	Definition of this word as an adjective
//
// More crucial TODO: implement portability, that is at least comment extended toolkit where it's not avaliable for some reasons (WINDOWS)
//
// Distant TODO: implement encounter counter and possibility to sort by it
//
// Most distant TODO: implement task-based or thread-pool-based concurrent beforehand web-api processing of non-capitalized words

template <typename J>
std::set<StringType> dig_to_definition(const J &json_table)
{
	std::set<StringType> definitions;

	if (json_table.contains("definition"))
	{
		definitions.emplace(json_table["definition"]);
	}

	for (const auto &level: json_table)
	{
		if (level.is_object() || level.is_array())
		{
			auto current = dig_to_definition(level);
			definitions.merge(current);
		}
	}

	return definitions;
}

std::set<StringType> find_word_definition(const StringType &word)
{
	StringType dictionaryapi_request{ "https://api.dictionaryapi.dev/api/v2/entries/en/" };
	StringType exact_request_address = dictionaryapi_request + word;

	cpr::Response response = cpr::Get(cpr::Url{ exact_request_address }, cpr::VerifySsl(false));

	auto some_json = json::parse(response.text);

	return dig_to_definition(some_json);
}

class DictionaryCreator
{
	using DictionaryEntryType = StringType;
#ifdef __THIS_IS_WINDOWS__
	std::basic_regex<CharType> word_pattern{ LR"([[:alpha:]]{2,})" };
	std::basic_regex<CharType> name_pattern{ LR"([^\s\.-]+\s*([A-Z][a-z]+))" };
	std::basic_regex<CharType> name_at_the_start{ LR"(^([A-Z][a-z]+).*$)" };
	StringType terminating_characters{ L".!?" };
#else
	std::basic_regex<CharType> word_pattern{ R"([[:alpha:]]{2,})" };
	std::basic_regex<CharType> name_pattern{ R"([^\s\.-]+\s*([A-Z][a-z]+))" };
	std::basic_regex<CharType> name_at_the_start{ R"(^([A-Z][a-z]+).*$)" };
	StringType terminating_characters{ ".!?" };
#endif
public:
	DictionaryCreator(StringType dir = StringType{}) : directory{ dir }
	{
		output << "Constructed a dictionary creator in directory \"" << directory << "\"\n";
	}
	
	void request_the_extensions()
	{
		auto possible_extensions = get_possible_extensions();

		output << "There are files of " << possible_extensions.size() << " extensions:\n";

		for (const auto &s : possible_extensions)
		{
			output << "\t" << s << std::endl;
		}
		
		output << "Specify which of them to parse for the dictionary.\nExample: .txt or simply txt\n"
				<< "Use " << input_terminator_string << " to terminate input\n";
		StringType ext;
		while (input >> ext && ext != input_terminator_string)
		{
			if (*ext.begin() != '.')
			{
				ext.insert(ext.begin(), '.');
			}
			text_files_extensions.insert(ext);
		}

		output << "Parsing files of following extensions:\n";
		for (const auto &s : text_files_extensions)
		{
			output << "\t" << s << std::endl;
		}
	}
	
	std::set<StringType> get_possible_extensions() const
	{
		std::set<StringType> extensions;
				
		for (auto &p : fs::directory_iterator(directory))
		{
			extensions.emplace(p.path().extension().c_str());
		}

		return extensions;
	}
	
	void parse_all_files()
	{
		for (auto &p : fs::directory_iterator(directory))
		{
			if (text_files_extensions.find(p.path().extension()) != text_files_extensions.end())
			{
				FileInputStream current_file(p.path());
				parse_to_dictionary(std::move(current_file));
			}
		}
	}

	void parse_to_dictionary(FileInputStream file_input)
	{
		if (file_input)
		{
			size_t string_number = 0;
			StringType current_string;
			bool previous_string_terminated = true;

			while (std::getline(file_input, current_string))
			{
				std::match_results<StringType::const_iterator> matches;

				for (auto it = std::regex_iterator<StringType::const_iterator>(current_string.begin(), current_string.end(), word_pattern);
					it != std::regex_iterator<StringType::const_iterator>{}; ++it)
				{
					matches = *it;
					dictionary[upper_case_letter(matches[0].str().front())].emplace(matches[0].str());
				}

				for (auto it = std::regex_iterator<StringType::const_iterator>(current_string.begin(), current_string.end(), name_pattern);
					it != std::regex_iterator<StringType::const_iterator>{}; ++it)
				{
					matches = *it;
					proper_nouns[matches[1].str().front()].emplace(matches[1].str());
				}

				if (std::regex_match(current_string, matches, name_at_the_start))
				{
					proper_nouns[matches[1].str().front()].emplace(matches[1].str());
				}

				if (current_string.find_last_of(terminating_characters) > current_string.find_last_not_of(terminating_characters))
				{
					previous_string_terminated = true;
				}
				else
				{
					previous_string_terminated = false;
				}
			}
		}
	}

	void export_dictionary(StringType dest_name = DefaultDictionaryName)
	{
		FileOutputStream file_output(dest_name);

		if (file_output.good())
		{
			output << "Exporting dictionary to " << dest_name << std::endl;
		}
		else
		{
			output << "Cannot export dictionary to " << dest_name << std::endl;
			return;
		}

		remove_proper_nouns();

		for (auto const &letter : dictionary)
		{
			if (!letter.second.empty())
			{
				file_output << "\n\n\t" << letter.first << "\n---------------\n";
				for (auto const &entry : letter.second)
				{
					file_output << entry << "\n";
					auto definitions = find_word_definition(entry);
					for (const auto &d: definitions)
					{
						file_output << "\t" << d << "\n";
					}
					if (!definitions.empty())
					{
						file_output << std::endl;
					}
				}
				file_output << "---------------\n";
			}
		}
	}

	void remove_proper_nouns()
	{
		for (const auto &pn : proper_nouns)
		{
			CharType letter = pn.first;

			for (const auto &word : pn.second)
			{
				dictionary[letter].erase(word);
			}
		}

		output << "Had to remove from the dictionary following proper nouns:\n";
		for (const auto &letter : proper_nouns)
		{
			output << letter.first << ": ";
			for (const auto &w : letter.second)
			{
				output << "\t" << w << "\n";
			}
			output << std::endl;
		}
	}

private:
	StringType directory;
	std::set<StringType> text_files_extensions;

	std::map<CharType, std::set<DictionaryEntryType>> dictionary;
	std::map<CharType, std::set<StringType>> proper_nouns;
};

#ifdef __THIS_IS_UNIX__
int main
#else
int wmain
#endif
(int argc, CharType **argv)
{
	setlocale(LC_ALL, "Russian_Russia.1251");

	output << "DictionaryCreator 0.5\n";

	if (argc > 1)
	{
		return argc;
	}

	StringType current_directory{ argv[0] };
	StringType slash;

#ifdef __THIS_IS_WINDOWS__
	slash = LR"(\/)";
#else
	slash = R"(\/)";
#endif

	current_directory.erase(current_directory.find_last_of(slash), StringType::npos);

	DictionaryCreator dc(current_directory);
	dc.request_the_extensions();
	dc.parse_all_files();
	dc.export_dictionary();

	return 0;
}
