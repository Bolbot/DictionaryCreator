#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#define NOMINMAX

#include <vector>
#include <iterator>
#include <exception>
#include <algorithm>
#include <istream>
#include <ostream>
#include <iostream>
#include <iomanip>
#include <cctype>

#include "dictionary_types.h"
#include "dictionary_entry.h"
#include "dictionary_language.h"

namespace dictionary_creator
{
	constexpr size_t maximum_rand_output = RAND_MAX;

	class DictionaryExporter;			// Tight coupled with Dictionary class, defined below

	enum class ComparisonType : size_t
	{
		MostFrequent = 0, LeastFrequent = 1, Longest = 2,
		Shortest = 3, MostAmbiguous = 4, LeastAmbiguous = 5
	};

	using entry_sorter_t = std::function<bool(const Entry *, const Entry *)>;

	size_t utf8_length(const utf8_string &string) noexcept;

	static const std::vector<entry_sorter_t> criteria_dependent_sorters
	{
		[] (const Entry *a, const Entry *b) { return a->get_counter() > b->get_counter(); },
		[] (const Entry *a, const Entry *b) { return a->get_counter() < b->get_counter(); },
		[] (const Entry *a, const Entry *b) { return utf8_length(a->get_word()) > utf8_length(b->get_word());  },
		[] (const Entry *a, const Entry *b) { return utf8_length(a->get_word()) < utf8_length(b->get_word());  },
		[] (const Entry *a, const Entry *b)
		{
			size_t a_total = 0;
			for (const auto &i: a->get_definitions())
			{
				a_total += i.second.size();
			}

			size_t b_total = 0;
			for (const auto &i: b->get_definitions())
			{
				b_total += i.second.size();
			}
			return a_total > b_total;
		},
		[] (const Entry *a, const Entry *b)
		{
			size_t a_total = 0;
			for (const auto &i: a->get_definitions())
			{
				a_total += i.second.size();
			}

			size_t b_total = 0;
			for (const auto &i: b->get_definitions())
			{
				b_total += i.second.size();
			}
			return a_total < b_total;
		}
	};

	class Dictionary
	{
	public:
		Dictionary(Language language);
		Dictionary &merge(const Dictionary &other);
		Dictionary &merge(Dictionary &&other);
		Dictionary &subtract(const Dictionary &other);
		Dictionary &subtract(Dictionary &&other);
		Dictionary intersection_with(const Dictionary &other) const;

		bool add_word(utf8_string word);
		bool remove_word(utf8_string word);

		void add_proper_noun(utf8_string proper_noun);

		const Entry *const lookup(utf8_string word) const;

		size_t total_words() const;

		std::vector<const Entry *> get_top(ComparisonType criterion, size_t quantity) const;
		std::vector<const Entry *> get_letter_entries(letter_type letter) const;
		std::vector<const Entry *> get_undefined() const;
		const Entry *get_random_word() const;
		std::vector<const Entry *> get_random_words(size_t number) const;

		letter_type get_first_letter(utf8_string word) const;

		Dictionary &operator+=(const Dictionary &other);
		Dictionary &operator+=(Dictionary &&other);
		Dictionary &operator-=(const Dictionary &other);
		Dictionary &operator-=(Dictionary &&other);
		Dictionary &operator*=(const Dictionary &other);
		
		friend class DictionaryExporter;

		Language get_language() const noexcept;
	private:
		void remove_proper_nouns();

		Language language;
		std::map<letter_type, std::set<Entry>> dictionary;
		std::map<letter_type, std::set<Entry>> proper_nouns;
	};

	Dictionary operator+(Dictionary left, const Dictionary &right);
	Dictionary operator+(Dictionary left, Dictionary &&right);
	Dictionary operator-(Dictionary left, const Dictionary &right);
	Dictionary operator-(Dictionary left, Dictionary &&right);
	Dictionary operator*(const Dictionary &left, const Dictionary &right);

	size_t random_number(size_t max = maximum_rand_output);

	enum class ExportOptions : size_t
	{
		Dictionary = 1,
		ProperNouns = (1 << 1),

		DefinedWords = (1 << 2),
		UndefinedWords = (1 << 3),
		UndefinedWarnings = (1 << 4),

		OnlyOneDefinition = (1 << 5),
		EveryPartOfSpeech = (1 << 6),
		EncountersInText = (1 << 7),

		DashedList = (1 << 8),
		NumberedList = (1 << 9),

		BasicDecorations = (1 << 10),
		AdvancedDecorations = (1 << 11)
	};

	constexpr ExportOptions operator|(ExportOptions a, ExportOptions b)
	{
		return static_cast<ExportOptions>(static_cast<size_t>(a) | static_cast<size_t>(b));
	}
	constexpr size_t operator&(ExportOptions a, ExportOptions b)
	{
		return static_cast<size_t>(a) & static_cast<size_t>(b);
	}

	constexpr ExportOptions default_export_options = ExportOptions::Dictionary	
		| ExportOptions::DefinedWords | ExportOptions::UndefinedWords 
		| ExportOptions::EveryPartOfSpeech | ExportOptions::DashedList;

	class DictionaryExporter
	{
	public:
		explicit DictionaryExporter(std::ostream &output_stream = std::cout, utf8_string undefined_warning = u8"-----");
		explicit DictionaryExporter(std::ostream *output_stream = nullptr, utf8_string undefined_warning = u8"-----");

		std::ostream &export_dictionary(Dictionary &object, ExportOptions export_options = default_export_options);
		std::ostream &export_entries(const std::vector<const Entry *> &entries, ExportOptions export_options = default_export_options);

	private:
		void print_letter(letter_type letter, ExportOptions options);
		void print_entry(const Entry &entry, ExportOptions options);
		void print_empty_line(ExportOptions options);

		std::ostream *output_stream;
		utf8_string undefined_warning;
	};
}

#endif
