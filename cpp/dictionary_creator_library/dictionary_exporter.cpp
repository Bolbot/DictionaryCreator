#include "dictionary_exporter.h"

#include "dictionary_types.h"

#include <iomanip>

dictionary_creator::DictionaryExporter::DictionaryExporter(std::ostream* output_stream, utf8_string undefined_warning)
	: output_stream{ output_stream }, undefined_warning{ undefined_warning }
{
	if (output_stream == nullptr)
	{
		throw dictionary_creator::dictionary_runtime_error("DictionaryExporter hasn't been properly initialized");
	}
}

std::ostream& dictionary_creator::DictionaryExporter::export_dictionary(const dictionary_creator::Dictionary& object,
	dictionary_creator::ExportOptions options)
{
	if (!output_stream->good())
	{
		return *output_stream;
	}

	if ((options & ExportOptions::Dictionary) != 0)
	{
		for (const auto& [letter, words] : object.get_main_dictionary())
		{
			if (words.empty())
			{
				continue;
			}

			print_letter(letter, options);

			for (const auto& word : words)
			{
				print_entry(*word, options);
			}

			print_empty_line(options);
		}
	}
	else if ((options & ExportOptions::ProperNouns) != 0)
	{
		for (const auto& [letter, entries] : object.get_proper_nouns_dictionary())
		{
			for (const auto& name : entries)
			{
				print_entry(*name, options);
			}
		}
	}

	return *output_stream;
}

std::ostream& dictionary_creator::DictionaryExporter::export_entries(const dictionary_creator::subset_t& entries, dictionary_creator::ExportOptions options)
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
		auto len = letter.size() * 10;
		*output_stream << std::setw(len) << std::setfill('-') << '\n'
			<< '|' << std::setw(len / 2) << std::setfill(' ') << letter << std::setw((len / 2) - 2) << "|\n"
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

void dictionary_creator::DictionaryExporter::print_entry(const dictionary_creator::Entry& entry, dictionary_creator::ExportOptions options)
{
	if (entry.is_defined() == false)
	{
		if (options & ExportOptions::UndefinedWords)
		{
			*output_stream << entry;

			if (options & dictionary_creator::ExportOptions::Length)
			{
				*output_stream << " (" << dictionary_creator::utf8_length(entry.get_word()) << ")";
			}
			if (options & dictionary_creator::ExportOptions::Frequency)
			{
				*output_stream << " [" << entry.get_counter() << "]";
			}

			if (options & dictionary_creator::ExportOptions::UndefinedWarnings)
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
		if (options & dictionary_creator::ExportOptions::Length)
		{
			*output_stream << " (" << dictionary_creator::utf8_length(entry.get_word()) << ")";
		}
		if (options & dictionary_creator::ExportOptions::Frequency)
		{
			*output_stream << " [" << entry.get_counter() << "]";
		}
		if (options & dictionary_creator::ExportOptions::Ambiguousness)
		{
			size_t total_definitions = 0;
			for (const auto& [part_of_speech, definitions] : entry.get_definitions())
			{
				for (const auto& def : definitions)
				{
					total_definitions += def.size();
				}
			}
			*output_stream << " { " << total_definitions << " }";
		}

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
					for (const auto& [part, definitions] : entry.get_definitions())
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
					for (const auto& [part, definitions] : entry.get_definitions())
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
					for (const auto& [part, definitions] : entry.get_definitions())
					{
						for (const auto& d : definitions)
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
					for (const auto& [part, definitions] : entry.get_definitions())
					{
						for (const auto& d : definitions)
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
