#ifndef __DICTIONARY_MANAGER_H__
#define __DICTIONARY_MANAGER_H__

#include "dictionary.h"
#include "dictionary_creator.h"
#include "dictionary_definer.h"

#include <filesystem>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>

// This class contains all the dictionary related features, that is
// 	- Dictionary class to store the dictionary itself
// 	- DictionaryCreator class responsible for parsing given UTF-8 aware streams according to language rules
// 	- DictionaryExporter class to export given dictionary to the user provided streams or ordered files

namespace dictionary_creator
{
	struct dictionary_filename
	{
		utf8_string full;
		std::string_view human_readable;
	};

	class DictionaryManager;

	DictionaryManager load_dictionary(utf8_string dictionary_filename);
	std::vector<dictionary_filename> available_dictionaries();

	class DictionaryManager
	{
	public:
		DictionaryManager(Language language);

		template <typename CharType>
		void add_input_file(std::basic_string<CharType> file_name)
		{
			std::ifstream stream(file_name);
			add_input_file(std::move(stream));
		}
		void add_input_file(std::ifstream &&file_stream);
		void parse_all_pending();
		void parse_one_line(utf8_string line);

		template <typename T, typename ... Arg>
		std::shared_ptr<Entry> lookup_or_add_word(utf8_string word, Arg &&... arg)
		{
			dictionary.add_word<T>(word, std::forward<Arg>(arg)...);
			return dictionary.lookup(std::move(word));
		}
		
		std::shared_ptr<Entry> lookup_or_add_word(utf8_string word);
		bool contains_word(utf8_string word) const;

		subset_t get_subset(ComparisonType criterion, size_t number) const;
		subset_t get_subset(letter_type letter, size_t number = 0) const;

		template <typename Comparator>
		subset_t get_subset(Comparator &&comparator, size_t number) const
		{
			return dictionary.get_top(std::function(std::forward<Comparator>(comparator)), number);
		}

		subset_t get_undefined(size_t number = 0) const;
		std::shared_ptr<Entry> get_random_word() const;
		subset_t get_random_words(size_t number) const;

		std::shared_ptr<Entry> define(utf8_string word);
		std::shared_ptr<Entry> define(std::shared_ptr<Entry> word);
		std::shared_ptr<Entry> define(const Entry &word);
		const subset_t &define(const subset_t &entries);

		void set_output(std::ostream &output_stream);

		void export_dictionary(ExportOptions = default_export_options);
		void export_top(ComparisonType criterion, size_t top, ExportOptions = default_export_options);
		void export_subset(const subset_t &subset, ExportOptions = default_export_options);

		void rename(utf8_string new_name);
		utf8_string get_name() const noexcept;
		void save_dictionary() const;
		friend DictionaryManager load_dictionary(utf8_string dictionary_filename);
		friend std::vector<dictionary_filename> available_dictionaries();

	private:
		utf8_string name = "Default dictionary";
		definer_t definer;
		Dictionary dictionary;
		DictionaryCreator creator;
		DictionaryExporter exporter;

		static constexpr auto dictionaries_directory = "Saved dictionaries";
		static constexpr auto dictionaries_extension = ".dic";
	};
}

#endif
