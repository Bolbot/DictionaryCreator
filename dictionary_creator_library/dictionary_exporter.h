#pragma once

#include "dictionary.h"

#include <cstdint>
#include <ostream>

namespace dictionary_creator
{
	enum class ExportOptions : size_t
	{
		Dictionary = 1,
		ProperNouns = (1 << 1),

		DefinedWords = (1 << 2),
		UndefinedWords = (1 << 3),
		UndefinedWarnings = (1 << 4),

		OnlyOneDefinition = (1 << 5),
		EveryPartOfSpeech = (1 << 6),
		EncountersInText = (1 << 7),

		DashedList = (1 << 8),
		NumberedList = (1 << 9),

		BasicDecorations = (1 << 10),
		AdvancedDecorations = (1 << 11),

		Frequency = (1 << 12),
		Length = (1 << 13),
		Ambiguousness = (1 << 14)
	};

	constexpr ExportOptions operator|(ExportOptions a, ExportOptions b)
	{
		return static_cast<ExportOptions>(static_cast<size_t>(a) | static_cast<size_t>(b));
	}
	constexpr size_t operator&(ExportOptions a, ExportOptions b)
	{
		return static_cast<size_t>(a) & static_cast<size_t>(b);
	}

	constexpr ExportOptions default_export_options = ExportOptions::Dictionary
		| ExportOptions::DefinedWords | ExportOptions::UndefinedWords
		| ExportOptions::EveryPartOfSpeech | ExportOptions::DashedList;

	class DictionaryExporter
	{
	public:
		explicit DictionaryExporter(std::ostream* output_stream = nullptr, utf8_string undefined_warning = u8"-----");

		std::ostream& export_dictionary(const Dictionary& object, ExportOptions export_options = default_export_options);
		std::ostream& export_entries(const subset_t& entries, ExportOptions export_options = default_export_options);

	private:
		void print_letter(letter_type letter, ExportOptions options);
		void print_entry(const Entry& entry, ExportOptions options);
		void print_empty_line(ExportOptions options);

		std::ostream* output_stream;
		utf8_string undefined_warning;
	};

}