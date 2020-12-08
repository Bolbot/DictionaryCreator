#include "dictionary_creator.h"


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

	auto some_json = json::parse(response);

	return set_of_definitions(some_json);
}

std::set<std::pair<StringType, std::set<StringType>>> dict::find_word_per_part_of_speech_definitions(const StringType &word)
{
	auto response = connections::lookup_online_dictionary(word);

	auto some_json = json::parse(response);

	return set_of_part_of_speech_definitions(some_json);
}

dict::DictionaryCreator::DictionaryCreator(FSStringType dir) : directory{ dir }
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
			std::smatch matches;

			for (auto it = std::sregex_iterator(current_string.begin(), current_string.end(), word_pattern);
				it != std::sregex_iterator{}; ++it)
			{
				matches = *it;
				dictionary[toupper(matches[0].str().front())].emplace(matches[0].str());
			}

			for (auto it = std::sregex_iterator(current_string.begin(), current_string.end(), name_pattern);
				it != std::sregex_iterator{}; ++it)
			{
				matches = *it;
				proper_nouns[matches[1].str().front()].emplace(matches[1].str());
			}

			if (std::regex_match(current_string, matches, name_at_the_start))
			{
				proper_nouns[matches[1].str().front()].emplace(matches[1].str());
			}

			if (current_string.find_last_of(terminating_characters) > current_string.find_last_not_of(terminating_characters))
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

	// TODO: supply this in separate file maybe
	output << "Had to remove from the dictionary following proper nouns:\n";
	for (const auto &letter : proper_nouns)
	{
		output << letter.first << ": ";
		for (const auto &w : letter.second)
		{
			output << "\t" << w << "\n";
		}
		output << std::endl;
	}
	
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
			for (const auto &pair: set)
			{
				for (const auto &definition: pair.second)
				{
					file_output << "\t(" << pair.first << "):\t"
						<< definition << "\n";
				}
			}
			if (!set.empty())
			{
				file_output << std::endl;
			}
			break;
		}
	}
}
