#include "dictionary_creator.h"

dictionary_creator::DictionaryCreator::DictionaryCreator(Language language)
	:
	language{ language },
	minimal_substantial_word_length{ dictionary_creator::minimal_substantial_word_length[static_cast<size_t>(language)] },
	terminating_characters{ dictionary_creator::terminating_characters[static_cast<size_t>(language)] },
	proper_nouns_extractor
	{
		pcre_parser::RegexParser(dictionary_creator::utf8_string
			{
				dictionary_creator::utf8_string{ u8R"((*UTF8)(?<=[^)" }
				+ terminating_characters
				+ dictionary_creator::utf8_string{ u8R"(]\s))" }
				+ dictionary_creator::utf8_string{ "([" }
				+ dictionary_creator::uppercase_letters[static_cast<size_t>(language)]
				+ dictionary_creator::utf8_string{ "][" }
				+ dictionary_creator::lowercase_letters[static_cast<size_t>(language)]
				+ dictionary_creator::utf8_string{ "]+)" }
			}.c_str())
	},
	linestarting_name_extractor
	{
		pcre_parser::RegexParser(dictionary_creator::utf8_string
			{
			[language]
			{
				dictionary_creator::utf8_string res =
					dictionary_creator::utf8_string{ u8R"((*UTF8)^([)" }
					+ dictionary_creator::uppercase_letters[static_cast<size_t>(language)]
					+ dictionary_creator::utf8_string{ "][" }
					+ dictionary_creator::lowercase_letters[static_cast<size_t>(language)]
					+ dictionary_creator::utf8_string{ u8R"(]+){1}\s?.*$)" };
				return res;
			}()
			}.c_str())
	},
	words_extractor
	{
		pcre_parser::RegexParser(dictionary_creator::utf8_string
			{
			[language, this]
			{
		       		dictionary_creator::utf8_string res = 
					dictionary_creator::utf8_string{ u8R"(\b([)" }
					+ dictionary_creator::uppercase_letters[static_cast<size_t>(language)]
					+ "]["
					+ dictionary_creator::lowercase_letters[static_cast<size_t>(language)]
					+ dictionary_creator::utf8_string{ "]{" }
					+ std::to_string(minimal_substantial_word_length - 1)
					+ dictionary_creator::utf8_string{ ",}" }
					+ u8R"()\b|\b([)"
					+ dictionary_creator::lowercase_letters[static_cast<size_t>(language)]
					+ dictionary_creator::utf8_string{ "]{" }
					+ std::to_string(minimal_substantial_word_length)
					+ dictionary_creator::utf8_string{ u8R"(,})\b)" };
				return res;
			}()
			}.c_str())
	}
{}

void dictionary_creator::DictionaryCreator::add_input(std::unique_ptr<std::istream> &&uptr_to_stream)
{
	if (uptr_to_stream != nullptr)
	{
		input_files.push(std::move(uptr_to_stream));
	}
}

dictionary_creator::Dictionary dictionary_creator::DictionaryCreator::parse_to_dictionary()
{
	dictionary_creator::Dictionary result(language);

	while (!input_files.empty())
	{
		auto &current = input_files.front();
		if (current->good())
		{
			result.merge(parse_one_file(*current));
		}
		input_files.pop();
	}

	return result;
}

dictionary_creator::Dictionary dictionary_creator::DictionaryCreator::parse_line(dictionary_creator::utf8_string line) const
{
	dictionary_creator::Dictionary dictionary(language);

	remove_crlf(line);

	if (dictionary_creator::utf8_length(line) >= minimal_substantial_word_length)
	{
		auto found_words = words_extractor.all_matches(line);
		for (auto &word: found_words)
		{
			dictionary.add_word(word);
		}

		auto found_names = proper_nouns_extractor.all_matches(line);
		for (auto &name: found_names)
		{
			dictionary.add_proper_noun(name);
		}
	}

	return dictionary;
}

dictionary_creator::Dictionary dictionary_creator::DictionaryCreator::parse_one_file(std::istream &file_input)
{
	dictionary_creator::Dictionary dictionary(language);

	bool previous_string_terminated = true;
	utf8_string current_string;

	while (std::getline(file_input, current_string))
	{
		dictionary.merge(parse_line(current_string));

		if (previous_string_terminated == false)
		{
			if (auto first_name = linestarting_name_extractor.single_match(current_string); !first_name.empty())
			{
				dictionary.add_proper_noun(first_name);
			}
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
