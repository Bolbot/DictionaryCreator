#ifndef __DICTIONARY_CREATOR_H__
#define __DICTIONARY_CREATOR_H__

// Stupid windows.h is messing with min name, so following disables their dumb macros
#define NOMINMAX

#include <set>
#include <map>
#include <regex>
#include <utility>
#include <fstream>
#include <optional>
#include <algorithm>

#include "utils.h"
#include "connections.h"
#include "regex_parser.h"

namespace dict
{
	template <typename J>
	std::optional<StringType> single_definition_impl(const J &json_table)
	{
		if (json_table.contains("definition"))
		{
#if __STRINGTYPE_IS_WSTRING__			
			return converter_to_wstring.from_bytes(json_table["definition"]);
#else
			return json_table["definition"];
#endif			
		}
		else if (json_table.is_object() || json_table.is_array())
		{
			for (const auto &part : json_table)
			{
				if (auto res = single_definition_impl(part); res)
				{
					return res.value();
				}
			}
		}

		return {};
	}

	template <typename J>
	StringType single_definition(const J &json_table)
	{
#if __STRINGTYPE_IS_WSTRING__
		static const StringType no_definition = L"no definition found";
#else
		static const StringType no_definition = "no definition found";
#endif
		return single_definition_impl(json_table).value_or(no_definition);
	}

	template <typename J>
	std::set<StringType> set_of_definitions(const J &json_table)
	{
		std::set<StringType> definitions;

		if (json_table.contains("definition"))
		{
#if __STRINGTYPE_IS_WSTRING__
			definitions.emplace(converter_to_wstring.from_bytes(json_table["definition"]));
#else
			definitions.emplace(json_table["definition"]);
#endif
		}

		for (const auto &level : json_table)
		{
			if (level.is_object() || level.is_array())
			{
				auto current = set_of_definitions(level);
				definitions.merge(current);
			}
		}

		return definitions;
	}
	
	template <typename J>
	std::map<StringType, std::set<StringType>> part_of_speech_definitions_map(const J &json_table)
	{
		std::map<StringType, std::set<StringType>> map;

		if (json_table.contains("partOfSpeech"))
		{
#if __STRINGTYPE_IS_WSTRING__
			StringType current_part_of_speech = converter_to_wstring.from_bytes(json_table["partOfSpeech"]);
#else
			StringType current_part_of_speech = json_table["partOfSpeech"];
#endif
			auto current_definitions = set_of_definitions(json_table);
			map[current_part_of_speech] = std::move(current_definitions);
		}

		for (const auto &level : json_table)
		{
			if (level.is_object() || level.is_array())
			{
				auto current = part_of_speech_definitions_map(level);
				map.merge(current);
			}
		}

		return map;
	}

	class Entry
	{
	public:
		using CharType = wchar_t;
		using POS_definitions = std::map<StringType, std::set<StringType>>;
		Entry(WordType<CharType> the_word = WordType<CharType>{});
		Entry(StringType the_word);

		void increment() const;
		size_t get_counter() const;

		void find_definitions() const;

		WordType<CharType> get_word() const;
		const POS_definitions &get_definitions() const;
	private:
		WordType<CharType> word;
		mutable POS_definitions definitions;
		mutable size_t encounters{ 1 };
	};

	bool operator<(const Entry &a, const Entry &b);
	
	enum class export_data { OneDefinition, AllDefinitions, AllDefinitionsPerPartOfSpeech };

	class DictionaryCreator
	{
		using SorterType = std::function<bool(const Entry *, const Entry *)>;
		const size_t minimal_substantial_string_length = 3;
		const char *const name_lookbehind_pattern = R"((?<=[^.!?]\s)([A-Z][a-z]+))";
		pcre_parser::RegexParser proper_nouns_extractor;
#if __STRINGTYPE_IS_WSTRING__
		StringType terminating_characters{ L".!?" };
		using CharType = wchar_t;
		std::wregex word_pattern{ LR"([[:alpha:]]{3,})" };
		std::wregex name_pattern{ LR"(([,[:alpha:]]+[[:space:]])+?([A-Z][a-z]+))" };
		std::wregex name_at_the_start{ LR"(^([A-Z][a-z]+){1}\s?.*$)" };
#else
		StringType terminating_characters{ ".!?" };
		using CharType = char;
		std::regex word_pattern{ R"([[:alpha:]]{3,})" };
		std::regex name_pattern{ R"(([,[:alpha:]]+[[:space:]])+?([A-Z][a-z]+))" };
		std::regex name_at_the_start{ R"(^([A-Z][a-z]+){1}\s?.*$)" };
#endif

	public:
		enum class ComparationType : size_t
		{
			MostFrequent = 0, LeastFrequent = 1, Longest = 2, Shortest = 3, MostAmbiguous = 4, LeastAmbiguous = 5
		};

		DictionaryCreator();

		void parse_to_dictionary(InputStream &&file_input);

		void define_all_words();

		void export_dictionary(OutputStream &&file_output, export_data export_options = export_data::AllDefinitions);

		void export_proper_nouns(OutputStream &&file_output);

		std::vector<const Entry *> sort_some(ComparationType comparation_type, size_t top = 10);

		void export_top(ComparationType comparation_type, OutputStream &&file_output, size_t top = 10);

		void remove_proper_nouns();

	private:

		void print_content(OutputStream &file_output, const Entry &entry, export_data export_options);

		std::map<CharType, std::set<Entry>> dictionary;
		bool dictionary_defined{ false };					// TODO: this flag belongs to each Entry
		std::map<char, std::set<StringType>> proper_nouns;

		static const std::vector<SorterType> sorters;
	};

	void remove_crlf(StringType &string);
}


#endif
