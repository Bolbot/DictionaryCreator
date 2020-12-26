#ifndef __DICTIONARY_CREATOR_H__
#define __DICTIONARY_CREATOR_H__

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
			return json_table["definition"];
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
		return single_definition_impl(json_table).value_or("no definition found");
	}

	template <typename J>
	std::set<StringType> set_of_definitions(const J &json_table)
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
				auto current = set_of_definitions(level);
				definitions.merge(current);
			}
		}

		return definitions;
	}

	template <typename J>
	std::set<std::pair<StringType, std::set<StringType>>> set_of_part_of_speech_definitions(const J &json_table)
	{
		std::set<std::pair<StringType, std::set<StringType>>> set;

		if (json_table.contains("partOfSpeech"))
		{
			StringType current_part_of_speech = json_table["partOfSpeech"];
			auto current_definitions = set_of_definitions(json_table);
			set.emplace(std::make_pair<StringType, std::set<StringType>>(std::move(current_part_of_speech), std::move(current_definitions)));
		}

		for (const auto &level: json_table)
		{
			if (level.is_object() || level.is_array())
			{
				auto current = set_of_part_of_speech_definitions(level);
				set.merge(current);
			}
		}

		return set;
	}

	template <typename J>
	std::map<StringType, std::set<StringType>> part_of_speech_definitions_map(const J &json_table)
	{
		std::map<StringType, std::set<StringType>> map;

		if (json_table.contains("partOfSpeech"))
		{
			StringType current_part_of_speech = json_table["partOfSpeech"];
			auto current_definitions = set_of_definitions(json_table);
			map[current_part_of_speech] = std::move(current_definitions);
		}

		for (const auto &level: json_table)
		{
			if (level.is_object() || level.is_array())
			{
				auto current = part_of_speech_definitions_map(level);
				map.merge(current);
			}
		}

		return map;
	}

	StringType find_word_definition(const StringType &word);

	std::set<StringType> find_word_definitions(const StringType &word);

	std::set<std::pair<StringType, std::set<StringType>>> find_word_per_part_of_speech_definitions(const StringType &word);

	class Entry
	{
	public:
		using POS_definitions = std::map<StringType, std::set<StringType>>;
		Entry(WordType the_word = WordType{});
		void find_definitions();
		WordType get_word() const;
		const POS_definitions &get_definitions() const;
	private:
		WordType word;
		POS_definitions definitions;
	};
	
	enum class export_data { OneDefinition, AllDefinitions, AllDefinitionsPerPartOfSpeech };

	class DictionaryCreator
	{
		using DictionaryEntryType = StringType;
		std::regex word_pattern{ R"([[:alpha:]]{3,})" };
		std::regex name_pattern{ R"(([,[:alpha:]]+[[:space:]])+?([A-Z][a-z]+))" };
		const char *const name_lookbehind_pattern = R"((?<=[^.!?]\s)([A-Z][a-z]+))";
		std::regex name_at_the_start{ R"(^([A-Z][a-z]+){1}\s?.*$)" };
		pcre_parser::RegexParser proper_nouns_extractor;
		StringType terminating_characters{ ".!?" };
	public:
		DictionaryCreator(FSStringType dir = FSStringType{});
	
		void request_the_extensions();
		
		std::set<FSStringType> get_possible_extensions() const;

		void parse_all_files();

		void parse_to_dictionary(FileInputStream file_input);

		void export_dictionary(StringType dest_name = StringType{ "output_of_DictionaryCreator.txt" },
					export_data export_options = export_data::AllDefinitions);

		void export_proper_nouns(StringType dest_name = StringType{ "proper_nouns_by_DictionaryCreator.txt" });

		void remove_proper_nouns();

	private:
		FSStringType directory;
		std::set<FSStringType> text_files_extensions;

		std::map<char, std::set<DictionaryEntryType>> dictionary;
		std::map<char, std::set<StringType>> proper_nouns;
		
	};

	void print_content(FileOutputStream &file_output, StringType entry, export_data export_options);

	void remove_crlf(StringType &string);
}

#endif
