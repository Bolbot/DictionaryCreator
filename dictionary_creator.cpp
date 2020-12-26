#include "dictionary_creator.h"

// FOR DEBUG
#include <thread>


StringType dict::find_word_definition(const StringType &word)
{
	auto response = connections::lookup_online_dictionary(word);

	auto some_json = json::parse(response);

	return single_definition(some_json);
}

std::set<StringType> dict::find_word_definitions(const StringType &word)
{
	StringType dictionaryapi_request{ "https://api.dictionaryapi.dev/api/v2/entries/en/" };
	StringType exact_request_address = dictionaryapi_request + word;

	auto response = connections::get(exact_request_address);

	auto some_json = parse_json(response);

	return set_of_definitions(some_json);
}

std::set<std::pair<StringType, std::set<StringType>>> dict::find_word_per_part_of_speech_definitions(const StringType &word)
{
	auto response = connections::lookup_online_dictionary(word);

	auto some_json = parse_json(response);

	return set_of_part_of_speech_definitions(some_json);
}

dict::Entry::Entry(WordType the_word) : word{ the_word }
{}

void dict::Entry::find_definitions()
{
	auto response = connections::lookup_online_dictionary(word);

	auto some_json = parse_json(response);

	definitions = part_of_speech_definitions_map(some_json);
}

WordType dict::Entry::get_word() const
{
	return word;
}

const dict::Entry::POS_definitions &dict::Entry::get_definitions() const
{
	return definitions;
}

dict::DictionaryCreator::DictionaryCreator(FSStringType dir) : directory{ dir }, proper_nouns_extractor{ name_lookbehind_pattern }
{
	output << "Constructed a dictionary creator in directory \"" << stdstring(directory) << "\"\n";
}

void dict::DictionaryCreator::request_the_extensions()
{
	auto possible_extensions = get_possible_extensions();

	output << "There are files of " << possible_extensions.size() << " extensions:\n";

	for (const auto &s : possible_extensions)
	{
		output << "\t" << stdstring(s) << std::endl;
	}
	
	auto input_terminator_string = "!";
	output << "Specify which of them to parse for the dictionary.\nExample: .txt or simply txt\n"
			<< "Use " << input_terminator_string << " to terminate input\n";
	StringType ext;
	while (input >> ext && ext != input_terminator_string)
	{
		if (*ext.begin() != '.')
		{
			ext.insert(ext.begin(), '.');
		}
		text_files_extensions.insert(to_fsstring(ext));
	}

	output << "Parsing files of following extensions:\n";
	for (const auto &s : text_files_extensions)
	{
		output << "\t" << stdstring(s) << std::endl;
	}
}

std::set<FSStringType> dict::DictionaryCreator::get_possible_extensions() const
{
	std::set<FSStringType> extensions;
			
	for (auto &p : fs::directory_iterator(directory))
	{
		extensions.emplace(p.path().extension().c_str());
	}

	return extensions;
}

void dict::DictionaryCreator::parse_all_files()
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

void dict::DictionaryCreator::parse_to_dictionary(FileInputStream file_input)
{
	if (file_input)
	{
		bool previous_string_terminated = true;
		StringType current_string;

		while (std::getline(file_input, current_string))
		{
			remove_crlf(current_string);

			if (current_string.empty() || current_string.size() < 3)
			{
				continue;
			}
			
			std::smatch matches;

			for (auto it = std::sregex_iterator(current_string.begin(), current_string.end(), word_pattern);
				it != std::sregex_iterator{}; ++it)
			{	
				matches = *it;
				dictionary[toupper(matches[0].str().front())].emplace(matches[0].str());
			}

			auto found_names = proper_nouns_extractor.all_matches(current_string);
			for (auto &name: found_names)
			{
				proper_nouns[name.front()].emplace(std::move(name));
			}

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
}

void dict::DictionaryCreator::export_dictionary(StringType dest_name, export_data export_options)
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


				print_content(file_output, entry, export_options);
			}
			file_output << "---------------\n";
		}
	}
}

void dict::DictionaryCreator::export_proper_nouns(StringType dest_name)
{
	FileOutputStream file_output(dest_name);

	if (file_output.good())
	{
		output << "Exporting proper nouns to " << dest_name << std::endl;
	}
	else
	{
		output << "Failed to export proper nouns to " << dest_name << std::endl;
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

void dict::DictionaryCreator::remove_proper_nouns()
{
	for (const auto &pn : proper_nouns)
	{
		auto letter = pn.first;

		for (const auto &word : pn.second)
		{
			dictionary[letter].erase(word);
		}
	}

	size_t proper_nouns_number = 0;
	for (const auto &letter : proper_nouns)
	{
		proper_nouns_number += letter.second.size();
	}

	output << "Had to remove from the dictionary " << proper_nouns_number << " proper nouns.\n";
}

void dict::print_content(FileOutputStream &file_output, StringType entry, dict::export_data export_options)
{
//	output << "Printing content of " << entry << std::endl;
	
	switch (export_options)
	{
	case export_data::OneDefinition:
		{
			auto definition = find_word_definition(entry);
			file_output << "\t" << definition << "\n";
			break;
		}
	case export_data::AllDefinitions:
		{
			auto definitions = find_word_definitions(entry);
			for (const auto &d: definitions)
			{
				file_output << "\t" << d << "\n";
			}
			if (!definitions.empty())
			{
				file_output << std::endl;
			}
			break;
		}
	case export_data::AllDefinitionsPerPartOfSpeech:
		{
			auto set = find_word_per_part_of_speech_definitions(entry);
			if (!set.empty())
			{
				for (const auto &pair : set)
				{
					file_output << "\t" << pair.first << ":\n";
					size_t i = 0;
					for (const auto &definition : pair.second)
					{
						++i;
						file_output << "\t\t\t\t";
						if (pair.second.size() > 1)
						{
							file_output << i << ") ";
						}
						file_output << definition << "\n";
					}
				}
				file_output << std::endl;
			}
			else
			{
				file_output << "\t\t\t\t----Undefined----" << std::endl;
			}
			break;
		}
	}
}

void dict::remove_crlf(StringType &string)
{
	while (!string.empty() && (string.back() == 0xA || string.back() == 0xD))
	{
		string.pop_back();
	}
}