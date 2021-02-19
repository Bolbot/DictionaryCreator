#include "dictionary.h"

size_t dictionary_creator::utf8_length(const dictionary_creator::utf8_string &string) noexcept
{
	size_t separate_characters = 0;
	for (auto i: string)
	{
		if ((i & 0xC0) != 0x80)
		{
			++separate_characters;
		}
	}
	return separate_characters;
}

dictionary_creator::Dictionary::Dictionary(dictionary_creator::Language language) : language{ language }
{}

dictionary_creator::Dictionary &dictionary_creator::Dictionary::merge(const dictionary_creator::Dictionary &other)
{
	if (this->language != other.language)
	{
		throw std::runtime_error("an attempt to merge language mismatching dictionaries");
	}
	
	for (const auto &[letter, entries]: other.dictionary)
	{
		for (const auto &entry: entries)
		{
			if (auto existing = dictionary[letter].find(entry); existing != dictionary[letter].end())
			{
				existing->increment_counter(entry.get_counter());
			}
			else
			{
				dictionary[letter].insert(entry);
			}
		}
	}

	for (const auto &[letter, entries]: other.proper_nouns)
	{
		for (const auto &entry: entries)
		{
			proper_nouns[letter].insert(entry);
		}
	}

	remove_proper_nouns();

	return *this;
}

dictionary_creator::Dictionary &dictionary_creator::Dictionary::merge(dictionary_creator::Dictionary &&other)
{
	if (language != other.language)
	{
		throw std::runtime_error("an attempt to merge language mismatching dictionaries");
	}

	for (auto &[letter, entries]: other.dictionary)
	{
		for (auto &entry: entries)
		{
			if (auto existing = dictionary[letter].find(entry); existing != dictionary[letter].end())
			{
				existing->increment_counter(entry.get_counter());
			}
		}
		dictionary[letter].merge(std::move(other.dictionary[letter]));
	}
	
	for (auto &[letter, entries]: other.proper_nouns)
	{
		proper_nouns[std::move(letter)].merge(std::move(entries));
	}

	remove_proper_nouns();

	return *this;
}

dictionary_creator::Dictionary &dictionary_creator::Dictionary::subtract(const dictionary_creator::Dictionary &other)
{
	if (language != other.language)
	{
		throw std::runtime_error("an attempt to subtract language mismatching dictionaries");
	}

	for (const auto &[letter, entries]: other.dictionary)
	{
		for (const auto &entry: entries)
		{
			dictionary[letter].erase(entry);
		}
	}

	for (const auto &[letter, entries]: other.proper_nouns)
	{
		for (const auto &entry: entries)
		{
			proper_nouns[letter].insert(entry);
		}
	}

	remove_proper_nouns();

	return *this;
}

dictionary_creator::Dictionary &dictionary_creator::Dictionary::subtract(dictionary_creator::Dictionary &&other)
{
	if (language != other.language)
	{
		throw std::runtime_error("an attempt to subtract language mismatching dictionaries");
	}

	for (auto &[letter, entries]: other.dictionary)
	{
		for (const auto &entry: entries)
		{
			dictionary[letter].erase(entry);
		}
	}

	for (auto &[letter, entries]: other.proper_nouns)
	{
		proper_nouns[letter].merge(std::move(entries));
	}

	remove_proper_nouns();

	return *this;
}

dictionary_creator::Dictionary dictionary_creator::Dictionary::intersection_with(const dictionary_creator::Dictionary &other) const
{
	if (language != other.language)
	{
		throw std::runtime_error("an attempt to find intersection of lanugage mismatching dictionaries");
	}

	dictionary_creator::Dictionary result(language);

	for (const auto &[letter, entries]: dictionary)
	{
		try
		{
			for (const auto &entry : entries)
			{
				if (other.dictionary.at(letter).find(entry) != other.dictionary.at(letter).end())
				{
						result.dictionary[letter].insert(entry.is_defined() ? entry : *other.dictionary.at(letter).find(entry));
				}
			}
		}
		catch (std::out_of_range &no_such_letter_which_is_ok) { }
	}

	result.proper_nouns = other.proper_nouns;
	for (const auto &[letter, entries]: proper_nouns)
	{
		for (const auto &entry: entries)
		{
			result.proper_nouns[letter].insert(entry);
		}
	}

	result.remove_proper_nouns();

	return result;
}

dictionary_creator::letter_type dictionary_creator::Dictionary::get_first_letter(utf8_string word) const
{
	letter_type first_letter;

	switch (language)
	{
	case Language::English:
		first_letter = std::toupper(static_cast<unsigned char>(word.front()));
		break;
	case Language::French:
		// TODO			break;
	case Language::Russian:
		// TODO			break;
	case Language::German:
		// TODO			break;
	default:
		throw std::runtime_error("Language is not supported");
	}

	return first_letter;
}

bool dictionary_creator::Dictionary::add_word(utf8_string word)
{
	letter_type first_letter = get_first_letter(word);

	dictionary_creator::Entry entry(std::move(word));

	if (auto word = dictionary[first_letter].find(entry); word == dictionary[first_letter].end())
	{
		auto [iterator, emplacement_happened] = dictionary[first_letter].emplace(std::move(entry));
		return emplacement_happened;
	}
	else
	{
		word->increment_counter();
		return false;
	}
}

bool dictionary_creator::Dictionary::remove_word(utf8_string word)
{
	letter_type first_letter = get_first_letter(word);

	return dictionary[first_letter].erase(dictionary_creator::Entry(std::move(word)));
}

void dictionary_creator::Dictionary::add_proper_noun(utf8_string proper_noun)
{
	letter_type first_letter = get_first_letter(proper_noun);

	proper_nouns[first_letter].emplace(std::move(proper_noun));
}

const dictionary_creator::Entry *const dictionary_creator::Dictionary::lookup(dictionary_creator::utf8_string word) const
{
	Entry entry(word);
	letter_type first_letter = get_first_letter(word);

	try
	{
		if (auto iterator = dictionary.at(first_letter).find(entry); iterator != dictionary.at(first_letter).end())
		{
			return &*iterator;
		}
	}
	catch (std::out_of_range &) {}

	return nullptr;
}

size_t dictionary_creator::Dictionary::total_words() const
{
	size_t result = 0;

	for (const auto &[letter, words]: dictionary)
	{
		result += words.size();
	}

	return result;
}

std::vector<const dictionary_creator::Entry *> dictionary_creator::Dictionary::get_top(dictionary_creator::ComparisonType criterion, size_t quantity) const
{
	size_t total_entries = total_words();

	quantity = std::min(quantity, total_entries);

	std::vector<const dictionary_creator::Entry *> entries;
	entries.reserve(total_entries);

	for (const auto &[letter, words]: dictionary)
	{
		for (const auto &w: words)
		{
			entries.push_back(&w);
		}
	}

	std::partial_sort(entries.begin(), entries.begin() + quantity, entries.end(),
			dictionary_creator::criteria_dependent_sorters[static_cast<size_t>(criterion)]);

	entries.resize(quantity);

	return entries;
}

std::vector<const dictionary_creator::Entry *> dictionary_creator::Dictionary::get_letter_entries(dictionary_creator::letter_type letter) const
{
	std::vector<const dictionary_creator::Entry *> entries;

	try
	{
		size_t total_entries = dictionary.at(letter).size();
		entries.reserve(total_entries);

		for (const auto &word: dictionary.at(letter))
		{
			entries.push_back(&word);
		}
	}
	catch (std::out_of_range &) {}

	return entries;
}

std::vector<const dictionary_creator::Entry *> dictionary_creator::Dictionary::get_undefined() const
{
	std::vector<const dictionary_creator::Entry *> result;

	for (const auto &[letter, entries]: dictionary)
	{
		for (const auto &entry: entries)
		{
			if (entry.is_defined() == false)
			{
				result.push_back(&entry);
			}
		}
	}

	return result;
}

const dictionary_creator::Entry *dictionary_creator::Dictionary::get_random_word() const
{
	if (dictionary.empty())
	{
		throw std::runtime_error("attempt to get random word from an empty dictionary");
	}

	auto letter = dictionary.begin();
	std::advance(letter, random_number(dictionary.size()));

	while (letter->second.empty())
	{
		letter = dictionary.begin();
		std::advance(letter, random_number(dictionary.size()));
	}

	auto word = letter->second.begin();
	std::advance(word, random_number(letter->second.size()));
	return &*word;
}

std::vector<const dictionary_creator::Entry *> dictionary_creator::Dictionary::get_random_words(size_t number) const
{
	number = std::min(number, total_words());

	std::set<const dictionary_creator::Entry *> set;

	while (set.size() != number)
	{
		set.insert(get_random_word());
	}

	return std::vector<const dictionary_creator::Entry *>(set.begin(), set.end());
}

size_t dictionary_creator::random_number(size_t max)
{
	return (max != 0 ? rand() % max : 0);
}

void dictionary_creator::Dictionary::remove_proper_nouns()
{
	for (const auto &[letter, entries]: proper_nouns)
	{
		for (const auto &word: entries)
		{
			dictionary[letter].erase(word);
		}
	}
}

dictionary_creator::Language dictionary_creator::Dictionary::get_language() const noexcept
{
	return language;
}

dictionary_creator::Dictionary &dictionary_creator::Dictionary::operator+=(const dictionary_creator::Dictionary &other)
{
	return merge(other);
}

dictionary_creator::Dictionary &dictionary_creator::Dictionary::operator+=(dictionary_creator::Dictionary &&other)
{
	return merge(std::move(other));
}

dictionary_creator::Dictionary &dictionary_creator::Dictionary::operator-=(const dictionary_creator::Dictionary &other)
{
	return subtract(other);
}

dictionary_creator::Dictionary &dictionary_creator::Dictionary::operator-=(dictionary_creator::Dictionary &&other)
{
	return subtract(std::move(other));
}

dictionary_creator::Dictionary &dictionary_creator::Dictionary::operator*=(const dictionary_creator::Dictionary &other)
{
	return *this = intersection_with(other);
}

dictionary_creator::Dictionary dictionary_creator::operator+(dictionary_creator::Dictionary left, const dictionary_creator::Dictionary &right)
{
	left.merge(right);
	return left;
}

dictionary_creator::Dictionary dictionary_creator::operator+(dictionary_creator::Dictionary left, dictionary_creator::Dictionary &&right)
{
	left.merge(std::move(right));
	return left;
}

dictionary_creator::Dictionary dictionary_creator::operator-(dictionary_creator::Dictionary left, const dictionary_creator::Dictionary &right)
{
	left.subtract(right);
	return left;
}

dictionary_creator::Dictionary dictionary_creator::operator-(dictionary_creator::Dictionary left, dictionary_creator::Dictionary &&right)
{
	left.subtract(std::move(right));
	return left;
}

dictionary_creator::Dictionary dictionary_creator::operator*(const dictionary_creator::Dictionary &left, const dictionary_creator::Dictionary &right)
{
	return left.intersection_with(right);
}

dictionary_creator::DictionaryExporter::DictionaryExporter(std::ostream &output_stream, utf8_string undefined_warning)
	: output_stream{ &output_stream }, undefined_warning{ undefined_warning }
{
	if (!output_stream.good())
	{
		throw std::runtime_error("output stream is not in good condition");
	}
}

dictionary_creator::DictionaryExporter::DictionaryExporter(std::ostream *output_stream, utf8_string undefined_warning)
	: dictionary_creator::DictionaryExporter{ output_stream ? *output_stream : std::cerr, undefined_warning }
{}

std::ostream &dictionary_creator::DictionaryExporter::export_dictionary(dictionary_creator::Dictionary &object,
	dictionary_creator::ExportOptions options)
{
	if (!output_stream->good())
	{
		return *output_stream;
	}

	if ((options & ExportOptions::Dictionary) != 0)
	{
		for (const auto &[letter, words] : object.dictionary)
		{
			if (words.empty())
			{
				continue;
			}

			print_letter(letter, options);

			for (const auto &word : words)
			{
				print_entry(word, options);
			}

			print_empty_line(options);
		}
	}
	else if ((options & ExportOptions::ProperNouns) != 0)
	{
		for (const auto &[letter, entries]: object.proper_nouns)
		{
			for (const auto &name: entries)
			{
				print_entry(name, options);
			}
		}
	}

	return *output_stream;
}

std::ostream &dictionary_creator::DictionaryExporter::export_entries(const std::vector<const dictionary_creator::Entry *> &entries,
		dictionary_creator::ExportOptions options)
{
	if (!output_stream->good())
	{
		return *output_stream;
	}

	for (auto i : entries)
	{
		print_entry(*i, options);
	}

	return *output_stream;
}

void dictionary_creator::DictionaryExporter::print_letter(dictionary_creator::letter_type letter, dictionary_creator::ExportOptions options)
{
	if (options & ExportOptions::BasicDecorations)
	{
		auto len = letter.size() * 5;
		*output_stream << std::setw(len) << std::setfill('-') << '\n'
			<< '|' << std::setw(len / 2) << std::setfill(' ') << letter << std::setw((len / 2) - 2) << '\n'
			<< std::setw(len) << std::setfill('-') << '\n';
	}
	else if (options & ExportOptions::AdvancedDecorations)
	{
		// TODO somtime later, in fact this is an unimportant makeup feature
	}
	else
	{
		*output_stream << letter << ":\n";
	}
}

void dictionary_creator::DictionaryExporter::print_entry(const dictionary_creator::Entry &entry, dictionary_creator::ExportOptions options)
{
	if (entry.is_defined() == false)
	{
		if (options & ExportOptions::UndefinedWords)
		{
			*output_stream << entry;
			if (options & ExportOptions::UndefinedWarnings)
			{
				*output_stream << "\t\t" << undefined_warning << std::endl;
			}
		}
	}
	else if ((options & ExportOptions::DefinedWords) == 0ull)
	{
		return;
	}
	else
	{
		*output_stream << entry;

		if ((options & ExportOptions::EveryPartOfSpeech) == 0ull)
		{
			if (options & ExportOptions::OnlyOneDefinition)
			{
				auto first_definition = entry.get_definitions().begin()->second.begin();
				*output_stream << u8" \U00002014 " << *first_definition;
			}
		}
		else
		{
			if (options & ExportOptions::OnlyOneDefinition)
			{
				if (options & ExportOptions::NumberedList)
				{
					size_t i = 1;
					for (const auto &[part, definitions] : entry.get_definitions())
					{
						*output_stream << "\t\t";

						if (entry.get_definitions().size() > 1)
						{
							*output_stream << i << ". ";
							++i;
						}
						*output_stream << "(" << part << ") " << *definitions.begin() << "\n";
					}
				}
				else
				{
					for (const auto &[part, definitions] : entry.get_definitions())
					{
						if (options & ExportOptions::DashedList)
						{
							*output_stream << u8"\t\t\U00002014 ";
						}
						*output_stream << *definitions.begin() << "\n";
					}
				}
			}
			else
			{
				if (options & ExportOptions::NumberedList)
				{
					size_t i = 1;
					for (const auto &[part, definitions] : entry.get_definitions())
					{
						for (const auto &d : definitions)
						{
							*output_stream << "\t\t";
							if (entry.get_definitions().size() > 1 || entry.get_definitions().begin()->second.size() > 1)
							{
								*output_stream << i << ". ";
								++i;
							}
							*output_stream << "(" << part << ") " << d << "\n";
						}
					}
				}
				else if (options & ExportOptions::DashedList)
				{
					for (const auto &[part, definitions] : entry.get_definitions())
					{
						for (const auto &d : definitions)
						{
							*output_stream << u8"\t\t\U00002014 (" << part << ") " << d << "\n";
						}
					}
				}
			}
		}
	}

	if (options & ExportOptions::EncountersInText)
	{
		*output_stream << "\t[" << entry.get_counter() << "]";
	}
	*output_stream << "\n";
}

void dictionary_creator::DictionaryExporter::print_empty_line(dictionary_creator::ExportOptions options)
{
	if (options & ExportOptions::BasicDecorations)
	{
		size_t len = 35;
		*output_stream << std::setw(len) << std::setfill('-') << "\n";
	}
	else if (options & ExportOptions::AdvancedDecorations)
	{
		// TODO later, maybe nothing to do whatsoever here
	}
	else
	{
		*output_stream << "\n";
	}
}
