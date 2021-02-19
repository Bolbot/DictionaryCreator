#include "dictionary_manager.h"

dictionary_creator::DictionaryManager::DictionaryManager(dictionary_creator::Language language) :
	definer{ [language] (dictionary_creator::utf8_string word) { return define_word(std::move(word), language); } },
	dictionary{ language },
	creator{ language },
	exporter{ std::cout, u8"----" }
{
	srand(time(nullptr));
}

void dictionary_creator::DictionaryManager::add_input_file(std::ifstream &&input_stream)
{
	creator.add_input(std::move(input_stream));
}

void dictionary_creator::DictionaryManager::add_input_file(std::string file_name)
{
	std::ifstream stream(file_name);
	if (stream.good())
	{
		creator.add_input(std::move(stream));
	}
}

void dictionary_creator::DictionaryManager::parse_all_pending()
{
	dictionary = creator.parse_to_dictionary();
}

const dictionary_creator::Entry *const dictionary_creator::DictionaryManager::lookup_or_add_word(dictionary_creator::utf8_string word)
{
	if (const auto *ptr = dictionary.lookup(word); ptr)
	{
		return ptr;
	}

	dictionary.add_word(word);

	return dictionary.lookup(word);
}

bool dictionary_creator::DictionaryManager::contains_word(dictionary_creator::utf8_string word) const
{
	return (dictionary.lookup(word) != nullptr);
}

std::vector<const dictionary_creator::Entry *> dictionary_creator::DictionaryManager::get_subset(dictionary_creator::ComparisonType criterion, size_t number)
{
	return dictionary.get_top(criterion, number);
}

std::vector<const dictionary_creator::Entry *> dictionary_creator::DictionaryManager::get_subset(dictionary_creator::letter_type letter, size_t number)
{
	auto res = dictionary.get_letter_entries(letter);

	if (number != 0 && res.size() > number)
	{
		res.resize(number);
	}

	return res;
}

std::vector<const dictionary_creator::Entry *> dictionary_creator::DictionaryManager::get_undefined(size_t number)
{
	auto res = dictionary.get_undefined();

	if (number != 0 && res.size() > number)
	{
		res.resize(number);
	}

	return res;
}

const dictionary_creator::Entry *dictionary_creator::DictionaryManager::get_random_word() const
{
	return dictionary.get_random_word();	
}

std::vector<const dictionary_creator::Entry *> dictionary_creator::DictionaryManager::get_random_words(size_t number)
{
	return dictionary.get_random_words(number);
}

const dictionary_creator::Entry *dictionary_creator::DictionaryManager::define(dictionary_creator::utf8_string word)
{
	const auto *entry = lookup_or_add_word(word);

	entry->define(definer);

	return entry;
}

const dictionary_creator::Entry *dictionary_creator::DictionaryManager::define(const dictionary_creator::Entry *word)
{
	word->define(definer);

	return word;
}

const dictionary_creator::Entry *dictionary_creator::DictionaryManager::define(const dictionary_creator::Entry &word)
{
	word.define(definer);

	return &word;
}

const std::vector<const dictionary_creator::Entry *> &dictionary_creator::DictionaryManager::define(const std::vector<const dictionary_creator::Entry *> &entries)
{
	for (const auto &i: entries)
	{
		i->define(definer);
	}

	return entries;
}

void dictionary_creator::DictionaryManager::set_output(std::ostream &output_stream)
{
	exporter = DictionaryExporter(output_stream, undefined_warnings[static_cast<size_t>(dictionary.get_language())]);
}

void dictionary_creator::DictionaryManager::export_dictionary(dictionary_creator::ExportOptions options)
{
	exporter.export_dictionary(dictionary, options);
}

void dictionary_creator::DictionaryManager::export_top(dictionary_creator::ComparisonType criterion, size_t top, dictionary_creator::ExportOptions options)
{
	exporter.export_entries(dictionary.get_top(criterion, top), options);
}

void dictionary_creator::DictionaryManager::export_subset(const std::vector<const dictionary_creator::Entry *> &subset,
								dictionary_creator::ExportOptions options)
{
	exporter.export_entries(subset, options);
}
