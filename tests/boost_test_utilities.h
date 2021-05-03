#ifndef BOOST_TEST_UTILITIES_H
#define BOOST_TEST_UTILITIES_H

#include "dictionary_types.h"
#include "dictionary_entry.h"
#include "regex_parser.h"

const auto unprocessed_json_string = R"([{"word":"frame","meanings": [ {"partOfSpeech":"verb","definitions":[ {"definition":"Place in a frame."}, {"definition":"Create or formulate"}, {"definition":"Produce false evidence"}, {"definition":"Erect the building."}]}, {"partOfSpeech":"noun","definitions":[ {"definition":"Structure that surrounds"}, {"definition":"A person's body"}, {"definition":"A basic structure"}, {"definition":"A structural environment"}, {"definition":"A single complete picture"}, {"definition":"A round of play in game."}, {"definition":"short for frame-up"}] }] }])";

constexpr auto DictionaryEntry_minimal_possible_size = sizeof(dictionary_creator::utf8_string)
	+ sizeof(dictionary_creator::definitions_t) + sizeof(size_t) + sizeof(bool);

constexpr auto Dictionary_minimal_possible_size = sizeof(size_t) + 2 
	* sizeof(std::map<dictionary_creator::letter_type, std::set<std::shared_ptr<dictionary_creator::Entry>>>);

constexpr auto DictionaryCreator_minimal_possible_size = sizeof(size_t) + sizeof(std::vector<std::unique_ptr<std::istream>>)
	+ sizeof(size_t) + sizeof(dictionary_creator::utf8_string) + 3 * sizeof(pcre_parser::RegexParser);

constexpr auto RegexParser_minimal_possible_size = sizeof(int) + sizeof(const char *) + sizeof(int) + sizeof(unsigned char) + sizeof(pcre*);

const std::initializer_list<dictionary_creator::utf8_string> english_words
{
	"arcbishop", "bomb", "cat", "dark", "enormous", "fly", "govern", "holy", "inactive", "jolly",
	"knight", "love", "misery", "nap", "over", "pun", "question", "rubish", "satellite", "thumb",
	"ubiquitous", "victory", "worm", "xeroradiography", "yield", "zeal"
};

const std::initializer_list<dictionary_creator::utf8_string> russian_words
{
	u8"атлет", u8"броня", u8"волк", u8"глава", u8"день", u8"если", u8"ёжик", u8"жесть", u8"зонтик",
	u8"истина", u8"йогурт", u8"каватина", u8"лучше", u8"метрополитен", u8"низина", u8"оружие",
	u8"профессионализм", u8"ручей", u8"силос", u8"танцевальный", u8"улучшение", u8"фанатично",
	u8"художник", u8"цыган", u8"честь", u8"шкаф", u8"щёлочь", u8"эротика", u8"юстиция", u8"ясно"
};

auto fake_definer = [] (auto whatever)
{
	dictionary_creator::definitions_t definitions;

	definitions["A"] = { { "B" }, { "C" }, { "D" } };
	definitions["One"] = { { "Two" }, { "Three" }, { "Four" } };

	return definitions;	
};

template <typename Container>
bool contains_word(const Container &container, const dictionary_creator::utf8_string &word)
{
	return std::find_if(container.begin(), container.end(),	[&word] (const auto &ptr) { return ptr->get_word() == word; }) != container.end();
};

class DifficultyEntry : public dictionary_creator::Entry
{
public:
	DifficultyEntry(dictionary_creator::utf8_string word, double difficulty = 1.0)
		: dictionary_creator::Entry{ std::move(word) }, difficulty{ difficulty }
	{}

	double get_difficulty() const
	{
		return difficulty;
	}
private:
	double difficulty;
};

auto get_comp_less_difficult()
{
	return std::function<bool(const DifficultyEntry &, const DifficultyEntry &)>
	{
		[](const DifficultyEntry &a, const DifficultyEntry &b)
		{
			return a.get_difficulty() < b.get_difficulty();
		} 
	};
}

auto get_comp_more_difficult()
{
	return std::function<bool(const DifficultyEntry &, const DifficultyEntry &)>
	{
		[](const DifficultyEntry &a, const DifficultyEntry &b)
		{
			return a.get_difficulty() > b.get_difficulty();
		} 
	};
}

bool exports_word(const dictionary_creator::Dictionary &dictionary, const dictionary_creator::utf8_string &word)
{
	std::stringstream stream;
	dictionary_creator::DictionaryExporter(&stream).export_dictionary(dictionary);

	return stream.str().find(word) != std::string::npos;
}

bool exports_words(const dictionary_creator::Dictionary &dictionary, const std::initializer_list<dictionary_creator::utf8_string> &words)
{
	std::stringstream stream;
	dictionary_creator::DictionaryExporter(&stream).export_dictionary(dictionary);

	for (const auto &i: words)
	{
		if (stream.str().find(i) == std::string::npos)
		{
			std::cout << "No \"" << i << "\"" << std::endl;
			return false;
		}
	}

	return true;
}

#endif
