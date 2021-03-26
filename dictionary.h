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
#include <array>
#include <type_traits>
#include <functional>
#include <cmath>

#include <boost/serialization/shared_ptr.hpp>

#include "dictionary_types.h"
#include "dictionary_entry.h"
#include "dictionary_language.h"

namespace dictionary_creator
{
	constexpr size_t maximum_rand_output = RAND_MAX;

	class DictionaryExporter;

	enum class ComparisonType : size_t
	{
		MostFrequent = 0, LeastFrequent = 1, Longest = 2,
		Shortest = 3, MostAmbiguous = 4, LeastAmbiguous = 5
	};

	using entry_sorter_t = const std::function<bool(const std::shared_ptr<Entry> &, const std::shared_ptr<Entry> &)>;

	static const std::array<entry_sorter_t, 6> criteria_dependent_sorters
	{
		[] (const std::shared_ptr<Entry> &a, const std::shared_ptr<Entry> &b) { return a->get_counter() > b->get_counter(); },
		[] (const std::shared_ptr<Entry> &a, const std::shared_ptr<Entry> &b) {	return a->get_counter() < b->get_counter(); },
		[] (const std::shared_ptr<Entry> &a, const std::shared_ptr<Entry> &b) { return utf8_length(a->get_word()) > utf8_length(b->get_word());	},
		[] (const std::shared_ptr<Entry> &a, const std::shared_ptr<Entry> &b) {	return utf8_length(a->get_word()) < utf8_length(b->get_word());	},
		[] (const std::shared_ptr<Entry> &a, const std::shared_ptr<Entry> &b)
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
		[] (const std::shared_ptr<Entry> &a, const std::shared_ptr<Entry> &b)
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

	class DefaultEntrySorter
	{
	public:
		bool operator()(const std::shared_ptr<Entry> &a, const std::shared_ptr<Entry> &b) const noexcept;

		using is_transparent = int;
		bool operator()(const std::shared_ptr<Entry> &a, const dictionary_creator::utf8_string &b) const noexcept;
		bool operator()(const dictionary_creator::utf8_string &a, const std::shared_ptr<Entry> &b) const noexcept;
	};

	template <typename T>
	using less_comp_t = std::function<bool(const T&, const T&)>;

	using subset_t = std::vector<std::shared_ptr<Entry>>;

	/*							// was it for debug purposes?
	template <typename T>
	void print_types(T &&t)
	{
		std::cout << typeid(t).name();
	}

	template <typename T, typename ...Args>
	void print_types(T &&t, Args &&... args)
	{
		std::cout << typeid(t).name() << ' ';
		print_types(std::forward<Args>(args)...);
	}
	*/

	class Dictionary
	{
	public:
		Dictionary(Language language);

		Dictionary &merge(const Dictionary &other);
		Dictionary &merge(Dictionary &&other);
		Dictionary &subtract(const Dictionary &other);
		Dictionary &subtract(Dictionary &&other);
		Dictionary intersection_with(const Dictionary &other) const;

		template <typename T, typename ... Args>
		bool add_word(utf8_string word, Args &&... args)
		{
			static_assert(std::is_base_of_v<Entry, T>, "User class isn't inherited from dictionary_creator::Entry");
	
			auto first_letter = get_first_letter(word);

			if (auto exists = dictionary[first_letter].find(std::make_shared<Entry>(word)); exists != dictionary[first_letter].end())
			{
				auto old_node = dictionary[first_letter].extract(exists);
				auto counter = old_node.value()->get_counter();

				old_node.value().reset(new T(std::move(word), std::forward<Args>(args)...));
				old_node.value()->increment_counter(counter - 1);
				dictionary[first_letter].insert(std::move(old_node));
			}
			else
			{
				auto [iterator, success] = dictionary[first_letter].insert(
						std::shared_ptr<Entry>{ new T(std::move(word), std::forward<Args>(args)...) });
				return success;
			}

			return false;
		}

		bool add_word(utf8_string word);
		bool remove_word(utf8_string word);
		void add_proper_noun(utf8_string proper_noun);

		std::shared_ptr<Entry> lookup(utf8_string word) const;
		size_t total_words() const;

		subset_t get_top(ComparisonType criterion, size_t quantity) const;

		template <typename T>
		subset_t get_top(less_comp_t<T> &&comparator, size_t quantity) const
		{
			static_assert(std::is_base_of_v<Entry, T>, "Comparator accepts unrelated to dictionary_creator::Entry types");

			auto hierarchy_aware_comp = [&comparator] (const std::shared_ptr<Entry> &a, const std::shared_ptr<Entry> &b)
			{
				const auto *aptr = dynamic_cast<const T *>(a.get());
				const auto *bptr = dynamic_cast<const T *>(b.get());

				if (aptr != nullptr && bptr != nullptr)
				{
					return comparator(*aptr, *bptr);
				}
				else if (aptr == nullptr && bptr == nullptr)
				{
					return a->get_word() < b->get_word();
				}

				return (aptr != nullptr);
			};

			size_t total = total_words();
			quantity = std::min(quantity, total);
			
			subset_t entries;
			entries.reserve(total);

			for (const auto &[letter, words]: dictionary)
			{
				for (const auto &word: words)
				{
					entries.push_back(word);
				}
			}

			std::partial_sort(entries.begin(), entries.begin() + quantity, entries.end(), hierarchy_aware_comp);

			entries.resize(quantity);

			return entries;
		}

		subset_t get_letter_entries(letter_type letter) const;
		subset_t get_undefined() const;
		std::shared_ptr<Entry> get_random_word() const;
		subset_t get_random_words(size_t number) const;

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
		std::map<letter_type, std::set<std::shared_ptr<Entry>, DefaultEntrySorter>> dictionary;
		std::map<letter_type, std::set<std::shared_ptr<Entry>, DefaultEntrySorter>> proper_nouns;

		friend class boost::serialization::access;

		template <typename A>
		void serialize(A &arch, const unsigned int version)
		{
			arch & language;
			arch & dictionary;
			arch & proper_nouns;
		}
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
		AdvancedDecorations = (1 << 11),

		Frequency = (1 << 12),
		Length = (1 << 13),
		Ambiguousness = (1 << 14)
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

		std::ostream &export_dictionary(const Dictionary &object, ExportOptions export_options = default_export_options);
		std::ostream &export_entries(const subset_t &entries, ExportOptions export_options = default_export_options);

	private:
		void print_letter(letter_type letter, ExportOptions options);
		void print_entry(const Entry &entry, ExportOptions options);
		void print_empty_line(ExportOptions options);

		std::ostream *output_stream;
		utf8_string undefined_warning;
	};
}

#endif
