#ifndef __DICTIONARY_MANAGER_H__
#define __DICTIONARY_MANAGER_H__

#include "dictionary.h"
#include "dictionary_creator.h"
#include "dictionary_definer.h"

// This class contains all the dictionary related features, that is
// 	- Dictionary class to store the dictionary itself
// 	- DictionaryCreator class responsible for parsing given UTF-8 aware streams according to language rules
// 	- DictionaryExporter class to export given dictionary to the user provided streams or ordered files

namespace dictionary_creator
{
	class DictionaryManager
	{
	public:
		DictionaryManager(Language language);

		void add_input_file(std::ifstream &&input_stream);
		void add_input_file(std::string file_name);
		void parse_all_pending();

		const Entry *const lookup_or_add_word(utf8_string word);
		bool contains_word(utf8_string word) const;
		std::vector<const Entry *> get_subset(ComparisonType criterion, size_t number);
		std::vector<const Entry *> get_subset(letter_type letter, size_t number = 0);
		std::vector<const Entry *> get_undefined(size_t number = 0);
		const Entry *get_random_word() const;
		std::vector<const Entry *> get_random_words(size_t number);

		const Entry *define(std::string word);
		const Entry *define(const Entry *word);
		const Entry *define(const Entry &word);
		const std::vector<const Entry *> &define(const std::vector<const Entry *> &entries);

		void set_output(std::ostream &output_stream);

		void export_dictionary(ExportOptions = default_export_options);
		void export_top(ComparisonType criterion, size_t top,
				ExportOptions = default_export_options);
		void export_subset(const std::vector<const Entry *> &subset,
				ExportOptions = default_export_options);

	private:
		definer_t definer;
		Dictionary dictionary;
		DictionaryCreator creator;
		DictionaryExporter exporter;
	};
}

#endif
