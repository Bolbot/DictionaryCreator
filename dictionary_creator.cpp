#include "dictionary_creator.h"

// FOR DEBUG
#include <thread>

dictionary_creator::DictionaryCreator::DictionaryCreator(Language language)
	:
	language{ language },
	terminating_characters{ dictionary_creator::terminating_characters[static_cast<size_t>(language)] },
	proper_nouns_extractor
	{
		pcre_parser::RegexParser(dictionary_creator::utf8_string
			{
					dictionary_creator::utf8_string{ u8R"((?<=[^)" }
					+ terminating_characters + 
					dictionary_creator::utf8_string{ u8R"(]\s))" }
					//+ dictionary_creator::general_name[static_cast<size_t>(language)] // following 5 lines instead
						+ dictionary_creator::utf8_string{ "([" }
						+ dictionary_creator::uppercase_letters[static_cast<size_t>(language)]
						+ dictionary_creator::utf8_string{ "][" }
						+ dictionary_creator::lowercase_letters[static_cast<size_t>(language)]
						+ dictionary_creator::utf8_string{ "]+)" }
			}.c_str())
	},
	minimal_substantial_word_length{ dictionary_creator::minimal_substantial_word_length[static_cast<size_t>(language)] },
	name_at_the_start
	{
       //	dictionary_creator::linestarting_names[static_cast<size_t>(language)]	// following 5 lines instead
		dictionary_creator::utf8_string{ u8R"(^([)" }
			+ dictionary_creator::uppercase_letters[static_cast<size_t>(language)]
			+ dictionary_creator::utf8_string{ "][" }
			+ dictionary_creator::lowercase_letters[static_cast<size_t>(language)]
		+ dictionary_creator::utf8_string{ u8R"(]+){1}\s?.*$)" }
	},
	word_pattern
	{
		//dictionary_creator::general_word[static_cast<size_t>(language)] + u8"{" + std::to_string(minimal_substantial_word_length) + u8",}"
		//	// following 6 lines instead
		dictionary_creator::utf8_string{ "[" }
		+ dictionary_creator::uppercase_letters[static_cast<size_t>(language)]
		+ dictionary_creator::lowercase_letters[static_cast<size_t>(language)]
		+ dictionary_creator::utf8_string{ "]{" }
		+ std::to_string(minimal_substantial_word_length)
		+ dictionary_creator::utf8_string{ ",}"}
	},
	name_pattern
	{
		//dictionary_creator::nonterminating[static_cast<size_t>(language)] + u8"+?" + dictionary_creator::general_name[static_cast<size_t>(language)]
		//	// following 8 lines instead
		dictionary_creator::utf8_string{ "([^" }
		+ terminating_characters
		+ dictionary_creator::utf8_string{ "]+[[:space:]])+?" }
		+ dictionary_creator::utf8_string{ "([" }
		+ dictionary_creator::uppercase_letters[static_cast<size_t>(language)]
		+ dictionary_creator::utf8_string{ "][" }
		+ dictionary_creator::lowercase_letters[static_cast<size_t>(language)]
		+ dictionary_creator::utf8_string{ "]+)" }
	}
{}

void dictionary_creator::DictionaryCreator::add_input(std::ifstream &&input_stream)
{
	input_files.push_back(std::move(input_stream));
}

dictionary_creator::Dictionary dictionary_creator::DictionaryCreator::parse_to_dictionary()
{
	dictionary_creator::Dictionary result(language);

	for (auto &input_stream: input_files)
	{
		if (input_stream.good())
		{
			dictionary_creator::Dictionary file_dictionary = parse_one_file(input_stream);
			result.merge(std::move(file_dictionary));
		}
		else
		{
			std::cerr << "Input not in a good state\n";
		}
	}

	return result;
}

dictionary_creator::Dictionary dictionary_creator::DictionaryCreator::parse_one_file(std::ifstream &file_input)
{
	dictionary_creator::Dictionary dictionary(language);

	bool previous_string_terminated = true;
	utf8_string current_string;

	while (std::getline(file_input, current_string))
	{
		remove_crlf(current_string);

		if (current_string.empty() || current_string.size() < minimal_substantial_word_length)
		{
			continue;
		}
			
		std::smatch matches;

		for (auto it = std::sregex_iterator(current_string.begin(), current_string.end(), word_pattern);
			it != std::sregex_iterator{}; ++it)
		{	
			matches = *it;
			dictionary.add_word(matches[0].str());
		}

		auto found_names = proper_nouns_extractor.all_matches(current_string);
		for (auto &name: found_names)
		{
			dictionary.add_proper_noun(name);
		}

		if (previous_string_terminated == false && std::regex_match(current_string, matches, name_at_the_start))
		{
			dictionary.add_proper_noun(matches[1].str());
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

	return dictionary;
}

void dictionary_creator::DictionaryCreator::remove_crlf(utf8_string &string) const
{
	while (!string.empty() && (string.back() == 0xA || string.back() == 0xD))
	{
		string.pop_back();
	}
}
