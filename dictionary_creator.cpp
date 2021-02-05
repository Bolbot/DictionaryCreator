#include "dictionary_creator.h"

// FOR DEBUG
#include <thread>

dict::Entry::Entry(WordType<CharType> the_word) : word{ the_word }
{}

void dict::Entry::increment() const
{
	++encounters;
}

size_t dict::Entry::get_counter() const
{
	return encounters;
}

dict::Entry::Entry(StringType the_word)
{
	word.reserve(the_word.size());
	for (auto c: the_word)
	{
		word.push_back(c);
	}
}

void dict::Entry::find_definitions() const
{
	std::wstring unconverted_word;
	unconverted_word.reserve(word.size());
	for (auto c : word)
	{
		unconverted_word.push_back(c);
	}
	std::string converted_word = converter_from_wstring.to_bytes(unconverted_word);

	auto response = connections::lookup_online_dictionary(converted_word.data());

	auto some_json = parse_json(response);

	definitions = part_of_speech_definitions_map(some_json);
}

WordType<dict::Entry::CharType> dict::Entry::get_word() const
{
	return word;
}

const dict::Entry::POS_definitions &dict::Entry::get_definitions() const
{
	return definitions;
}

bool dict::operator<(const dict::Entry &a, const dict::Entry &b)
{
	return a.get_word() < b.get_word();
}

const std::vector<dict::DictionaryCreator::SorterType> dict::DictionaryCreator::sorters
{
	[] (const Entry *a, const Entry *b) { return a->get_counter() > b->get_counter(); },
	[] (const Entry *a, const Entry *b) { return a->get_counter() < b->get_counter(); },
	[] (const Entry *a, const Entry *b) { return a->get_word().size() > b->get_word().size();  },
	[] (const Entry *a, const Entry *b) { return a->get_word().size() < b->get_word().size();  },
	// TODO: below is lazy unaccurate implementaion of most and least ambiguous words, reimplement
	[] (const Entry *a, const Entry *b) { return a->get_definitions().size() > b->get_definitions().size(); },
	[] (const Entry *a, const Entry *b) { return a->get_definitions().size() < b->get_definitions().size(); }
};

dict::DictionaryCreator::DictionaryCreator() : proper_nouns_extractor{ name_lookbehind_pattern }
{}

void dict::DictionaryCreator::parse_to_dictionary(InputStream &&file_input)
{

	bool previous_string_terminated = true;
	StringType current_string;

	while (std::getline(file_input, current_string))
	{
		remove_crlf(current_string);

		if (current_string.empty() || current_string.size() < minimal_substantial_string_length)
		{
			continue;
		}
#if __STRINGTYPE_IS_WSTRING__
		using IteratorType = std::wstring::const_iterator;
#else
		using IteratorType = std::string::const_iterator;
#endif
			
		std::match_results<IteratorType> matches;

		for (auto it = std::regex_iterator<IteratorType>(current_string.begin(), current_string.end(), word_pattern);
			it != std::regex_iterator<IteratorType>{}; ++it)
		{	
			matches = *it;
			auto first_letter = matches[0].str().front();

			if (auto word = dictionary[lowercase_letter(first_letter)].find(matches[0].str());
				word == dictionary[lowercase_letter(first_letter)].end())
			{
				dictionary[lowercase_letter(first_letter)].emplace(matches[0].str());
			}
			else
			{
				word->increment();
			}
		}

#if __STRINGTYPE_IS_WSTRING__
		auto converted_current_string = converter_from_wstring.to_bytes(current_string);
		auto found_names = proper_nouns_extractor.all_matches(converted_current_string);
		for (auto &name : found_names)
		{
			auto converted_name = converter_to_wstring.from_bytes(name);
			proper_nouns[converted_name.front()].emplace(std::move(converted_name));
		}
#else
		auto found_names = proper_nouns_extractor.all_matches(current_string);
		for (auto &name: found_names)
		{
			proper_nouns[name.front()].emplace(std::move(name));
		}
#endif

		if (previous_string_terminated == false && std::regex_match(current_string, matches, name_at_the_start))
		{
			proper_nouns[matches[1].str().front()].emplace(matches[1].str());
		}

		if (auto last_terminator = current_string.find_last_of(terminating_characters);
			last_terminator != std::string::npos &&
			last_terminator > current_string.find_last_not_of(terminating_characters))
		{
			previous_string_terminated = true;
		}
		else
		{
			previous_string_terminated = false;
		}
	}
}

void dict::DictionaryCreator::define_all_words()
{
	remove_proper_nouns();

	for (auto &[letter, words]: dictionary)
	{
		for (auto &word: words)
		{
			word.find_definitions();
		}
	}

	dictionary_defined = true;
}

void dict::DictionaryCreator::export_dictionary(OutputStream &&file_output, export_data export_options)
{
	if (!file_output.good())
	{
		error_output << "CAN NOT export dictionary, file output isn't good\n";
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
				//file_output << entry.get_word() << "\n";
				// TODO: provide another overload in utils.h for different underlying char types

				print_content(file_output, entry, export_options);
			}
			file_output << "---------------\n";
		}
	}
}


/*

void dict::DictionaryCreator::export_proper_nouns(OutputStream &&file_output)
{
	if (!file_output.good())
	{
		return;
	}

	for (const auto &letter : proper_nouns)
	{
		file_output << letter.first << ":\n";
		for (const auto &w: letter.second)
		{
			file_output << "\t" <<  w << "\n";
		}
		file_output << std::endl;
	}
}

std::vector<const dict::Entry *> dict::DictionaryCreator::sort_some(dict::DictionaryCreator::ComparationType comparation_type, size_t top)
{
	size_t words_total = 0;
	for (const auto &letter: dictionary)
	{
		words_total += letter.second.size();
	}
	top = std::min(words_total, top);

	std::vector<const Entry*> vector;
	vector.reserve(words_total);

	for (const auto &letter: dictionary)
	{
		for (const auto &word: letter.second)
		{
			vector.push_back(&word);
		}
	}

	std::partial_sort(vector.begin(), vector.begin() + top, vector.end(), sorters[static_cast<size_t>(comparation_type)]);

	vector.resize(top);

	return vector;
}


void dict::DictionaryCreator::export_top(dict::DictionaryCreator::ComparationType comparation_type, OutputStream &&file_output, size_t top)
{
	if (!file_output.good())
	{
		return;
	}

	auto pointers = sort_some(comparation_type, top);
	
	for (size_t i = 0; i != pointers.size(); ++i)
	{
		file_output << i + 1 << ". " << pointers[i]->get_word();
		switch (comparation_type)
		{
		case ComparationType::MostFrequent:
		case ComparationType::LeastFrequent:
			{
				if (auto counter = pointers[i]->get_counter(); counter > 1)
				{
					file_output << " (" << counter << ")";
				}
				break;
			}
		case ComparationType::Longest:
			{
				if (auto len = pointers[i]->get_word().size(); len > 5)
				{
					file_output << " (" << len << " letters)";
				}
				break;
			}
		case ComparationType::MostAmbiguous:
		case ComparationType::LeastAmbiguous:
			{
				size_t total_definitions = 0;
				size_t parts = 0;
				for (const auto &pos: pointers[i]->get_definitions())
				{
					total_definitions += pos.second.size();
					++parts;
				}

				if (total_definitions > 1)
				{
					file_output << "(" << total_definitions << " definitions as "
						<< parts << " parts of speech)";
				}

				break;
			}
		}
		file_output << std::endl;
	}
	file_output << std::endl;
}

*/

void dict::DictionaryCreator::remove_proper_nouns()
{
	for (const auto &pn : proper_nouns)
	{
		auto letter = pn.first;

		for (const auto &word : pn.second)
		{
			Entry removeit(word);
			dictionary[lowercase_letter(letter)].erase(removeit);
		}
	}

	size_t proper_nouns_number = 0;
	for (const auto &letter : proper_nouns)
	{
		proper_nouns_number += letter.second.size();
	}

	output << "Had to remove from the dictionary " << proper_nouns_number << " proper nouns.\n";
}

void dict::DictionaryCreator::print_content(OutputStream &file_output, const dict::Entry &entry, dict::export_data export_options)
{
	switch (export_options)
	{
		case export_data::AllDefinitionsPerPartOfSpeech:
		{
			if (!entry.get_definitions().empty())
			{
				for (const auto &pos: entry.get_definitions())
				{
					file_output << "\t" << pos.first << ":\n";
					size_t i = 0;
					for (const auto &definition: pos.second)
					{
						file_output << "\t\t\t\t";
						if (pos.second.size() > 1)
						{
							++i;
							file_output << i << ") ";
						}
						file_output << definition << "\n";
					}
				}
				file_output << std::endl;
			}
			else
			{
				if (dictionary_defined)
				{
					file_output << "\t\t\t\t----Undefined----" << std::endl;
				}
			}
			file_output << "\tEncountered ";
		       	switch (size_t encountered = entry.get_counter(); encountered)
			{
			case 1:
				file_output << "once" << std::endl;
				break;
			case 2:
				file_output << "twice" << std::endl;
				break;
			default:
				file_output << encountered << " times." << std::endl;
			}
		}
	}
}

void dict::remove_crlf(StringType &string)
{
	while (!string.empty() && (string.back() == 0xA || string.back() == 0xD))		// TODO: check this condition validity
	{
		string.pop_back();
	}
}
