#ifndef __DICTIONARY_LANGUAGE_H__
#define __DICTIONARY_LANGUAGE_H__

#include <string>
#include <vector>
#include <locale>
#include <exception>

#include "dictionary_types.h"

// This part is supposed to handle language issues like following:
//
// 	-- whether a word can start with a capital letter (german nouns can)
// 	-- how the sorting is done (ä is near a, not after z)
// 	-- how to specify the language for the dictionary - enum class Language that also serves as index in respective arrays
// 	-- how the first unicode letter is uppercased and returned
//
// 	-- consider creating a base class that is language-aware and has constant public member Language

namespace dictionary_creator
{
	enum class Language : size_t
	{
		English = 1, French = 2, Russian = 3, German = 4
	};

	inline size_t utf8_length(const utf8_string &string) noexcept
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

	inline letter_type first_letter(const utf8_string &word, Language language)
	{
		letter_type first_letter;

		switch (language)
		{
		case Language::English:
			first_letter = word.front();
			break;
		case Language::French:
		case Language::Russian:
		case Language::German:
		default:
			{
				first_letter = word.front();
				for (size_t i = 1; i != word.size(); ++i)
				{
					if ((word[i] & 0xC0) == 0x80)
					{
						first_letter.push_back(word[i]);
					}
					else
					{
						break;
					}
				}
				break;
			}
		}

		return first_letter;
	}

	inline letter_type uppercase_letter(letter_type letter, Language language)
	{
		switch (language)
		{
		case Language::English:
				return letter_type{ std::toupper(letter.front(), std::locale::classic()) };
			break;
		case Language::French:
			{
				if (letter.size() > 2)
				{
					throw std::runtime_error("broken UTF-8 letter");
				}

				if (letter.size() == 1)
				{
					return letter_type{ std::toupper(letter.front(), std::locale::classic()) };
				}

				size_t code = ((letter[0] & 0x1F) << 6) | (letter[1] & 0x3F);

				if (0xE0 <= code && code <= 0xFC)
				{
					code -= 0x20;
				}
				else if (code == 0xFF)
				{
					code = 0x178;
				}
				else if (code == 0x153)
				{
					code = 0x152;
				}

				letter[0] = (0xC0 | (code >> 0x6));
				letter[1] = (0x80 | (code & 0x3F));
				
				return letter;
			}
		case Language::Russian:
			{
				if (letter.size() != 2)
				{
					throw std::runtime_error("broken UTF-8 letter");
				}

				size_t code = ((letter[0] & 0x1F) << 6) | (letter[1] & 0x3F);

				if (0x430 <= code && code <= 0x44F)
				{
					code -= 0x20;
				}
				else if (code == 0x451)
				{
					code = 0x401;
				}

				letter[0] = (0xC0 | (code >> 0x6));
				letter[1] = (0x80 | (code & 0x3F));

				return letter;
			}
		case Language::German:
			{
				if (letter.size() > 2)
				{
					if (letter == u8"ẞ")
					{
						return letter;
					}
					else
					{
						throw std::runtime_error("broken UTF-8 letter");
					}
				}

				if (letter.size() == 1)
				{
					return letter_type{ std::toupper(letter.front(), std::locale::classic()) };
				}

				size_t code = ((letter[0] & 0x1F) << 6) | (letter[1] & 0x3F);

				if (0xE0 <= code && code <= 0xFC)
				{
					code -= 0x20;
				}
				else if (code == 0xE4)
				{
					code = 0xC4;
				}
				else if (code == 0xF6)
				{
					code = 0xD6;
				}
				else if (code == 0xFC)
				{
					code = 0xDC;
				}
				else if (code == 0xDF)
				{
					return dictionary_creator::letter_type{ u8"\U00001E9E" };
				}

				letter[0] = (0xC0 | (code >> 0x6));
				letter[1] = (0x80 | (code & 0x3F));
				
				return letter;
			}
		default:
			throw std::runtime_error("Language is not supported");
		}
	}

	const std::vector<language_code_t> language_codes
	{
		u8"", u8"en", u8"fr", u8"ru", u8"de"
	};

	const std::vector<std::string> undefined_warnings
	{
		u8"", u8"Definition is absent", u8"La définition est absente", u8"Определение отсутствует", u8"Definition ist nicht vorhanden"
	};

	const std::vector<size_t> minimal_substantial_word_length
	{
		0, 3, 3, 3, 3
	};

	const std::vector<std::string> terminating_characters
	{
		u8"", u8".?!", u8".!?", u8".?!", u8".?!"
	};

	const std::vector<utf8_string> uppercase_letters
	{
		u8"", u8"A-Z", u8"A-ZÀÂÆÇÈÉÊËÏÎÔŒÙŸ", u8"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", u8"A-ZÄÖÜ"
	};
	const std::vector<utf8_string> lowercase_letters
	{
		u8"", u8"a-z", u8"a-zàâéèêëïîôùûçœæ", u8"абвгдеёжзийклмнопрстуфхцчшщъыьэюя", u8"a-zäöüß"
	};
}

#endif
