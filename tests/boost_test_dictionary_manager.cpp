#define BOOST_TEST_MODULE Dictionary Manager Regress Test
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>

// following file inducing linking issues with x86 windows
// it's necessary in one block below which is ifdefed the same way
#if !defined WIN32 || defined _WIN64
#include <boost/test/tools/output_test_stream.hpp>
#endif

#include "dictionary_manager.h"

#include <array>
#include <algorithm>
#include <filesystem>

#include <iostream>

BOOST_AUTO_TEST_CASE(object_validity)
{
	constexpr auto DictionaryManager_minimal_possible_size =
		sizeof(dictionary_creator::utf8_string) +
		sizeof(dictionary_creator::definer_t) +
		sizeof(dictionary_creator::Dictionary) +
		sizeof(dictionary_creator::DictionaryCreator) +
		sizeof(dictionary_creator::DictionaryExporter);

	dictionary_creator::DictionaryManager manager(dictionary_creator::Language::English);
	BOOST_TEST_CHECK(sizeof(manager) >= DictionaryManager_minimal_possible_size);
	BOOST_TEST_CHECK(manager.contains_word(u8"complexity") == false);
	manager.lookup_or_add_word(u8"complexity");
	BOOST_TEST_CHECK(manager.contains_word(u8"complexity") == true);

	BOOST_TEST_CHECK(manager.get_name() == dictionary_creator::utf8_string{ "Default dictionary" });
	manager.rename(dictionary_creator::utf8_string{ "Simplest dictionary" });
	BOOST_TEST_CHECK(manager.get_name() == dictionary_creator::utf8_string{ "Simplest dictionary" });
}

BOOST_AUTO_TEST_CASE(simple_case)
{
	const auto test_line = u8"This is an example of english string. Not français. Русские слова не считываются."
	       "ALSO LET'S SAY This WAS SOME SORT OF NAME";

	dictionary_creator::DictionaryManager manager(dictionary_creator::Language::English);
	manager.parse_one_line(test_line);

	BOOST_TEST_CHECK(manager.contains_word("example"));
	BOOST_TEST_CHECK(manager.contains_word("english"));
	BOOST_TEST_CHECK(manager.contains_word("string"));
	BOOST_TEST_CHECK(manager.contains_word("This") == false);
	BOOST_TEST_CHECK(manager.contains_word(u8"français") == false);
	BOOST_TEST_CHECK(manager.contains_word(u8"слова") == false);
	BOOST_TEST_CHECK(manager.contains_word(u8"считываются") == false);

	dictionary_creator::DictionaryManager french(dictionary_creator::Language::French);
	french.parse_one_line(test_line);
	BOOST_TEST_CHECK(french.contains_word("example"));
	BOOST_TEST_CHECK(french.contains_word("english"));
	BOOST_TEST_CHECK(french.contains_word("string"));
	BOOST_TEST_CHECK(french.contains_word("This") == false);
	BOOST_TEST_CHECK(french.contains_word(u8"français"));
	BOOST_TEST_CHECK(french.contains_word(u8"слова") == false);
	BOOST_TEST_CHECK(french.contains_word(u8"считываются") == false);

	dictionary_creator::DictionaryManager russian(dictionary_creator::Language::Russian);
	russian.parse_one_line(test_line);
	BOOST_TEST_CHECK(russian.contains_word("example") == false);
	BOOST_TEST_CHECK(russian.contains_word("english") == false);
	BOOST_TEST_CHECK(russian.contains_word("string") == false);
	BOOST_TEST_CHECK(russian.contains_word("This") == false);
	BOOST_TEST_CHECK(russian.contains_word(u8"français") == false);
	BOOST_TEST_CHECK(russian.contains_word(u8"слова"));
	BOOST_TEST_CHECK(russian.contains_word(u8"считываются"));
}

struct SingleFile
{
	SingleFile()
	{
		BOOST_TEST_REQUIRE(std::ofstream(filename).good());
	}
	~SingleFile()
	{
		BOOST_TEST_CHECK(std::filesystem::remove(filename));
	}
	template <typename ... Args>
	void populate_file(Args &&... args) const
	{
		if (std::ofstream output(filename); output.good())
		{
			(output << ... << std::forward<Args>(args)) << std::endl;
		}
		else
		{
			BOOST_TEST_INFO("Failed to populate file");
			BOOST_TEST_CHECK(false);
		}
	}
	std::ifstream get_input() const
	{
		return std::ifstream(filename);
	}

	static constexpr auto filename = "boost_temporary_file_for_SingleFile_fixture.txt";
};

BOOST_FIXTURE_TEST_CASE(file_parsing, SingleFile)
{
	populate_file(u8"This is an example of english string. Not français. Русские слова не считываются.",
		"ALSO LET'S SAY This WAS SOME SORT OF NAME");

	dictionary_creator::DictionaryManager english(dictionary_creator::Language::English);
	english.add_input_file(get_input());
	english.parse_all_pending();

	BOOST_TEST_CHECK(english.contains_word("example"));
	BOOST_TEST_CHECK(english.contains_word("english"));
	BOOST_TEST_CHECK(english.contains_word("string"));
	BOOST_TEST_CHECK(english.contains_word("This") == false);
	BOOST_TEST_CHECK(english.contains_word(u8"français") == false);
	BOOST_TEST_CHECK(english.contains_word(u8"слова") == false);
	BOOST_TEST_CHECK(english.contains_word(u8"считываются") == false);

	dictionary_creator::DictionaryManager french(dictionary_creator::Language::French);
	french.add_input_file(get_input());
	french.parse_all_pending();

	BOOST_TEST_CHECK(french.contains_word("example"));
	BOOST_TEST_CHECK(french.contains_word("english"));
	BOOST_TEST_CHECK(french.contains_word("string"));
	BOOST_TEST_CHECK(french.contains_word("This") == false);
	BOOST_TEST_CHECK(french.contains_word(u8"français"));
	BOOST_TEST_CHECK(french.contains_word(u8"слова") == false);
	BOOST_TEST_CHECK(french.contains_word(u8"считываются") == false);

	dictionary_creator::DictionaryManager russian(dictionary_creator::Language::Russian);
	russian.add_input_file(dictionary_creator::utf8_string{ filename });
	russian.parse_all_pending();

	BOOST_TEST_CHECK(russian.contains_word("example") == false);
	BOOST_TEST_CHECK(russian.contains_word("english") == false);
	BOOST_TEST_CHECK(russian.contains_word("string") == false);
	BOOST_TEST_CHECK(russian.contains_word("This") == false);
	BOOST_TEST_CHECK(russian.contains_word(u8"français") == false);
	BOOST_TEST_CHECK(russian.contains_word(u8"слова"));
	BOOST_TEST_CHECK(russian.contains_word(u8"считываются"));
}

BOOST_FIXTURE_TEST_CASE(lookup, SingleFile)
{
	populate_file(u8"Das ist ein deutsches Beispiel. Werner hat ein barfuß Gefäß.");

	dictionary_creator::DictionaryManager german(dictionary_creator::Language::German);
	german.add_input_file(get_input());
	german.parse_all_pending();
	german.lookup_or_add_word(u8"reißen");
	german.lookup_or_add_word(u8"Grießbrei");

	BOOST_TEST_CHECK(german.contains_word(u8"deutsches"));
	//BOOST_TEST_CHECK(german.contains_word(u8"Beispiel"));
	BOOST_TEST_CHECK(german.contains_word(u8"barfuß"));
	//BOOST_TEST_CHECK(german.contains_word(u8"Gefäß"));
	BOOST_TEST_CHECK(german.contains_word(u8"reißen"));
	BOOST_TEST_CHECK(german.contains_word(u8"Grießbrei"));
	BOOST_TEST_CHECK(german.contains_word(u8"Defenestration") == false);
}

class PartOfSpeechEntry : public dictionary_creator::Entry
{
public:
	enum class Part_t : char
	{
		Noun, Verb, Pronoun, Adjective, Adverb,
		Conjunction, Preposition, Interjection
	};
	PartOfSpeechEntry(dictionary_creator::utf8_string word, Part_t part)
		: dictionary_creator::Entry{ std::move(word) }, part{ part }
	{}
	bool compares_with(const PartOfSpeechEntry &other) const noexcept
	{
		return part == other.part;
	}
private:
	Part_t part;
};

BOOST_AUTO_TEST_CASE(subtypes)
{
	dictionary_creator::DictionaryManager manager(dictionary_creator::Language::English);

	manager.lookup_or_add_word<PartOfSpeechEntry>("sight", PartOfSpeechEntry::Part_t::Verb);
	manager.lookup_or_add_word<PartOfSpeechEntry>("himself", PartOfSpeechEntry::Part_t::Pronoun);
	manager.lookup_or_add_word<PartOfSpeechEntry>("themselves", PartOfSpeechEntry::Part_t::Pronoun);

	BOOST_TEST_CHECK(manager.contains_word("sight"));
	BOOST_TEST_CHECK(manager.contains_word("himself"));
	BOOST_TEST_CHECK(manager.contains_word("themselves"));

	auto sight = dynamic_cast<PartOfSpeechEntry*>(manager.lookup_or_add_word("sight").get());
	auto himself = dynamic_cast<PartOfSpeechEntry*>(manager.lookup_or_add_word("himself").get());
	auto themselves = dynamic_cast<PartOfSpeechEntry*>(manager.lookup_or_add_word("themselves").get());

	BOOST_TEST_CHECK(sight != nullptr);
	BOOST_TEST_CHECK(himself != nullptr);
	BOOST_TEST_CHECK(sight->compares_with(*himself) == false);

	BOOST_TEST_CHECK(themselves != nullptr);
	BOOST_TEST_CHECK(themselves->compares_with(*himself));
}

const std::initializer_list<dictionary_creator::utf8_string> english_words
{
	"arcbishop", "bomb", "cat", "dark", "enormous", "fly", "govern", "holy", "inactive", "jolly",
	"knight", "love", "misery", "nap", "over", "pun", "question", "rubish", "satellite", "thumb",
	"ubiquitous", "victory", "worm", "xeroradiography", "yield", "zeal"
};

const std::array<dictionary_creator::utf8_string, 30> russian_words
{
	u8"атлет", u8"броня", u8"волк", u8"глава", u8"день", u8"если", u8"ёжик", u8"жесть", u8"зонтик",
	u8"истина", u8"йогурт", u8"каватина", u8"лучше", u8"метрополитен", u8"низина", u8"оружие",
	u8"профессионализм", u8"ручей", u8"силос", u8"танцевальный", u8"улучшение", u8"фанатично",
	u8"художник", u8"цыган", u8"честь", u8"шкаф", u8"щёлочь", u8"эротика", u8"юстиция", u8"ясно"
};

struct DictionaryManagerObjects
{
	DictionaryManagerObjects() :
		eng{ dictionary_creator::Language::English },
		rus{ dictionary_creator::Language::Russian }
	{
		for (auto i: english_words)
		{
			eng.lookup_or_add_word(i);
		}

		for (auto i: russian_words)
		{
			rus.lookup_or_add_word(i);
		}
	}

	dictionary_creator::DictionaryManager eng;
	dictionary_creator::DictionaryManager rus;
};

bool has_word(const dictionary_creator::subset_t &subset, dictionary_creator::utf8_string word)
{
	return std::find_if(subset.begin(), subset.end(), [&word] (auto x) { return x->get_word() == word; }) != subset.end();
}

BOOST_FIXTURE_TEST_CASE(subset_features, DictionaryManagerObjects)
{
	BOOST_TEST_CONTEXT("first letter")
	{
		BOOST_TEST_INFO("Each letter is represented by a single entry");

		for (auto letter: dictionary_creator::utf8_string{ "ABCDEFGhijklmnopQRSTUVWxyz" })
		{
			BOOST_TEST_INFO("get_subset(" << letter << ")");
			auto subset = eng.get_subset(dictionary_creator::letter_type(1, letter));
			BOOST_TEST_CHECK(subset.size() == 1u);
		}

		std::vector<dictionary_creator::utf8_string> letters =
		{
			"а", "б", "в", "г", "д", "е", "ё", "Ж", "З", "И", "Й", "К", "Л", "М",
			"н", "о", "п", "р", "с", "т", "У", "Ф", "Х", "ц", "ч", "ш", "Щ", "Э", "ю", "Я"
		};
		for (size_t i = 0; i != letters.size(); ++i)
		{
			BOOST_TEST_INFO("get_subset(" << letters[i] << ")");
			auto subset = rus.get_subset(letters[i]);
			BOOST_TEST_CHECK(subset.front()->get_word() == russian_words[i]);
			BOOST_TEST_CHECK(subset.size() == 1u);
		}
	}

	BOOST_TEST_CONTEXT("by length")
	{
		BOOST_TEST_CHECK(eng.get_subset(dictionary_creator::ComparisonType::Shortest, 3).size() == 3u);
		auto short4 = eng.get_subset(dictionary_creator::ComparisonType::Shortest, 4);

		BOOST_TEST_CHECK(has_word(short4, "cat"));
		BOOST_TEST_CHECK(has_word(short4, "nap"));
		BOOST_TEST_CHECK(has_word(short4, "fly"));
		BOOST_TEST_CHECK(has_word(short4, "pun"));

		BOOST_TEST_CHECK(eng.get_subset(dictionary_creator::ComparisonType::Longest, 1).front()->get_word() == "xeroradiography");

		BOOST_TEST_INFO("Top 0 returns empty structure");
		BOOST_TEST_CHECK(eng.get_subset(dictionary_creator::ComparisonType::Shortest, 0).size() == 0u);
		BOOST_TEST_CHECK(eng.get_subset(dictionary_creator::ComparisonType::Longest, 0).size() == 0u);

		BOOST_TEST_INFO("If requested number exceeds the size, all entries are returned sorted");
		auto english_shortest = eng.get_subset(dictionary_creator::ComparisonType::Shortest, 1'000'000);
		BOOST_TEST_CHECK(english_shortest.size() == 26u);
		BOOST_TEST_CHECK(std::is_sorted(english_shortest.begin(), english_shortest.end(),
				dictionary_creator::criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Shortest)]));

		auto english_longest = eng.get_subset(dictionary_creator::ComparisonType::Longest, 1'000'000);
		BOOST_TEST_CHECK(english_longest.size() == 26u);
		BOOST_TEST_CHECK(std::is_sorted(english_longest.begin(), english_longest.end(),
				dictionary_creator::criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Longest)]));
	}

	BOOST_TEST_CONTEXT("by ambiguity")
	{
		BOOST_TEST_CHECK(eng.define("love")->get_definitions().size() > 0u);
		BOOST_TEST_CHECK(eng.get_subset(dictionary_creator::ComparisonType::MostAmbiguous, 999).size() == 26u);
		BOOST_TEST_CHECK(has_word(eng.get_subset(dictionary_creator::ComparisonType::MostAmbiguous, 1), "love"));
		BOOST_TEST_CHECK(has_word(eng.get_subset(dictionary_creator::ComparisonType::LeastAmbiguous, 25), "love") == false);

		BOOST_TEST_INFO("Top 0 returns empty structure");
		BOOST_TEST_CHECK(eng.get_subset(dictionary_creator::ComparisonType::MostAmbiguous, 0).size() == 0u);
		BOOST_TEST_CHECK(eng.get_subset(dictionary_creator::ComparisonType::LeastAmbiguous, 0).size() == 0u);

		BOOST_TEST_INFO("If requested number exceeds the size, all entries are returned sorted");
		auto most = eng.get_subset(dictionary_creator::ComparisonType::MostAmbiguous, 1'000'000);
		BOOST_TEST_CHECK(most.size() == 26u);
		BOOST_TEST_CHECK(std::is_sorted(most.begin(), most.end(),
				dictionary_creator::criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostAmbiguous)]));

		auto least = eng.get_subset(dictionary_creator::ComparisonType::LeastAmbiguous, 1'000'000);
		BOOST_TEST_CHECK(least.size() == 26u);
		BOOST_TEST_CHECK(std::is_sorted(least.begin(), least.end(),
				dictionary_creator::criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastAmbiguous)]));
	}

	BOOST_TEST_CONTEXT("by frequency")
	{
		BOOST_TEST_CHECK(eng.get_subset(dictionary_creator::ComparisonType::MostFrequent, 1).size() == 1u);
		eng.lookup_or_add_word("ubiquitous")->increment_counter();
		eng.lookup_or_add_word("love")->increment_counter();
		BOOST_TEST_CHECK(has_word(eng.get_subset(dictionary_creator::ComparisonType::MostFrequent, 7), "ubiquitous") == true);
		BOOST_TEST_CHECK(has_word(eng.get_subset(dictionary_creator::ComparisonType::MostFrequent, 7), "love") == true);
		BOOST_TEST_CHECK(has_word(eng.get_subset(dictionary_creator::ComparisonType::LeastFrequent, 7), "ubiquitous") == false);
		BOOST_TEST_CHECK(has_word(eng.get_subset(dictionary_creator::ComparisonType::LeastFrequent, 7), "love") == false);

		BOOST_TEST_INFO("Top 0 returns empty structure");
		BOOST_TEST_CHECK(eng.get_subset(dictionary_creator::ComparisonType::MostFrequent, 0).size() == 0u);
		BOOST_TEST_CHECK(eng.get_subset(dictionary_creator::ComparisonType::LeastFrequent, 0).size() == 0u);

		BOOST_TEST_INFO("If requested number exceeds the size, all entries are returned sorted");
		auto most = eng.get_subset(dictionary_creator::ComparisonType::MostFrequent, 1'000'000);
		BOOST_TEST_CHECK(most.size() == 26u);
		BOOST_TEST_CHECK(std::is_sorted(most.begin(), most.end(),
				dictionary_creator::criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostFrequent)]));

		auto least = eng.get_subset(dictionary_creator::ComparisonType::LeastFrequent, 1'000'000);
		BOOST_TEST_CHECK(least.size() == 26u);
		BOOST_TEST_CHECK(std::is_sorted(least.begin(), least.end(),
				dictionary_creator::criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastFrequent)]));
	}

	BOOST_TEST_CONTEXT("undefined")
	{
		auto undefined = eng.get_undefined();
		BOOST_TEST_CHECK(undefined.size() == 25u);
		BOOST_TEST_CHECK(has_word(undefined, "love") == false);
		BOOST_TEST_CHECK(std::none_of(undefined.begin(), undefined.end(), [] (auto x) { return x->is_defined(); }));

		BOOST_TEST_CHECK(eng.get_undefined(19).size() == 19u);
		BOOST_TEST_CHECK(eng.get_undefined(1'000'021).size() == 25u);
		BOOST_TEST_CHECK(eng.get_undefined(25).size() == 25u);
	}

	BOOST_TEST_CONTEXT("random words")
	{
		BOOST_TEST_INFO("returns valid entry");
		BOOST_TEST_CHECK(rus.get_random_word().get() != nullptr);

		BOOST_TEST_INFO("number exact as requested or everything if request exceeds capacity");
		BOOST_TEST_CHECK(rus.get_random_words(11).size() == 11u);
		BOOST_TEST_CHECK(rus.get_random_words(0).size() == 0u);
		BOOST_TEST_CHECK(rus.get_random_words(1'000'000).size() == 30u);
	}
	
	BOOST_TEST_CONTEXT("custom comparator subsets")
	{
		auto softest_russian = [] (const dictionary_creator::Entry &a, const dictionary_creator::Entry &b) -> bool
		{
			auto a_word = a.get_word();
			auto b_word = b.get_word();

			auto a_last = a_word.substr(a_word.size() - 2);
			auto b_last = b_word.substr(b_word.size() - 2);

			auto is_vowel = [] (auto x) -> bool
			{
				return x == u8"а" || x == u8"e" || x == u8"и" || x == u8"о"
					|| x == u8"у" || x == u8"э" || x == u8"ю" || x == u8"я";
			};

			if (a_last == u8"ь")
			{
				return true;
			}
			else if (is_vowel(a_last))
			{
				return false;
			}
			else if (b_last == u8"ь")
			{
				return false;
			}
			else if (is_vowel(b_last))
			{
				return true;
			}

			return a_word < b_word;
		};

		auto five_softest = rus.get_subset(softest_russian, 5);

		BOOST_TEST_CHECK(five_softest.size() == 5u);
		BOOST_TEST_CHECK(has_word(five_softest, u8"день"));
		BOOST_TEST_CHECK(has_word(five_softest, u8"жесть"));
		BOOST_TEST_CHECK(has_word(five_softest, u8"честь"));
		BOOST_TEST_CHECK(has_word(five_softest, u8"щёлочь"));
		BOOST_TEST_CHECK(has_word(five_softest, u8"атлет"));
	}
	
}

BOOST_FIXTURE_TEST_CASE(definitions, DictionaryManagerObjects)
{
	eng.define("dark");
	auto question = eng.lookup_or_add_word("question");
	eng.define(question);
	auto victory = eng.lookup_or_add_word("victory");
	eng.define(*(victory.get()));

	auto two_longest = eng.get_subset(dictionary_creator::ComparisonType::Longest, 2);
	BOOST_TEST_CHECK(two_longest.front()->get_word() == "xeroradiography");
	BOOST_TEST_CHECK(two_longest.back()->get_word() == "ubiquitous");
	eng.define(two_longest);

	BOOST_TEST_CHECK(eng.lookup_or_add_word("dark")->is_defined());
	BOOST_TEST_CHECK(question->is_defined());
	BOOST_TEST_CHECK(victory->is_defined());
	BOOST_TEST_CHECK(eng.define("ubiquitous")->is_defined());
}

bool contains_strings(const std::stringstream &stream, std::initializer_list<dictionary_creator::utf8_string> list)
{
	for (auto i: list)
	{
		if (stream.str().find(i) == std::string::npos)
		{
			return false;
		}
	}
	return true;
}

#if !defined WIN32 || defined _WIN64
BOOST_FIXTURE_TEST_CASE(exporting, DictionaryManagerObjects)
{
	BOOST_TEST_CONTEXT("export dictionary actually exports something")
	{
		boost::test_tools::output_test_stream output;
		eng.set_output(output);
		BOOST_CHECK_NO_THROW(eng.export_dictionary());

		BOOST_TEST_CHECK(!output.is_empty());

		dictionary_creator::DictionaryManager french(dictionary_creator::Language::French);
		french.lookup_or_add_word(u8"garçon");
		french.set_output(output);
		french.export_dictionary();

		BOOST_TEST_CHECK(!output.is_empty(false));
		BOOST_TEST_CHECK(output.is_equal(u8"G:\ngarçon\n\n"));
	}

	BOOST_TEST_CONTEXT("empty exports nothing")
	{
		dictionary_creator::DictionaryManager empty(dictionary_creator::Language::German);
		boost::test_tools::output_test_stream output;
		empty.set_output(output);

		empty.export_dictionary();
		BOOST_TEST_CHECK(output.is_empty());
	}

	BOOST_TEST_CONTEXT("export dictionary exports all entries")
	{
		boost::test_tools::output_test_stream output;
		rus.set_output(output);
		rus.export_dictionary();

		BOOST_TEST_CHECK(output.is_equal(u8"А:\nатлет\n\nБ:\nброня\n\nВ:\nволк\n\nГ:\nглава\n\nД:\nдень\n\n"
					"Е:\nесли\n\nЁ:\nёжик\n\nЖ:\nжесть\n\nЗ:\nзонтик\n\nИ:\nистина\n\nЙ:\nйогурт\n\n"
					"К:\nкаватина\n\nЛ:\nлучше\n\nМ:\nметрополитен\n\nН:\nнизина\n\nО:\nоружие\n\n"
					"П:\nпрофессионализм\n\nР:\nручей\n\nС:\nсилос\n\nТ:\nтанцевальный\n\n"
					"У:\nулучшение\n\nФ:\nфанатично\n\nХ:\nхудожник\n\nЦ:\nцыган\n\nЧ:\nчесть\n\n"
					"Ш:\nшкаф\n\nЩ:\nщёлочь\n\nЭ:\nэротика\n\nЮ:\nюстиция\n\nЯ:\nясно\n\n"));
	}

	BOOST_TEST_CONTEXT("by length")
	{
		boost::test_tools::output_test_stream output;
		std::stringstream strings_output;

		eng.set_output(strings_output);
		eng.export_top(dictionary_creator::ComparisonType::Shortest, 4);
		BOOST_TEST_CHECK(contains_strings(strings_output, { "cat (3)\n", "fly (3)\n", "nap (3)\n", "pun (3)\n" }));
		strings_output.str({});

		eng.set_output(output);
		eng.export_top(dictionary_creator::ComparisonType::Longest, 1);
		BOOST_TEST_CHECK(output.is_equal("xeroradiography (15)\n"));

		BOOST_TEST_INFO("Top 0 exports nothing");
		eng.export_top(dictionary_creator::ComparisonType::Longest, 0);
		BOOST_TEST_CHECK(output.is_empty());

		BOOST_TEST_INFO("If requested number exceeds the size, simple export applies");
		rus.set_output(strings_output);
		rus.export_top(dictionary_creator::ComparisonType::Shortest, 1'000'000);
		BOOST_TEST_CHECK(contains_strings(strings_output, {
					u8"атлет (5)\n", u8"броня (5)\n", u8"волк (4)\n", u8"глава (5)\n", u8"день (4)\n", 
					u8"если (4)\n", u8"ёжик (4)\n", u8"жесть (5)\n", u8"зонтик (6)\n", u8"истина (6)\n", u8"йогурт (6)\n", 
					u8"каватина (8)\n", u8"лучше (5)\n", u8"метрополитен (12)\n", u8"низина (6)\n", u8"оружие (6)\n", 
					u8"профессионализм (15)\n", u8"ручей (5)\n", u8"силос (5)\n", u8"танцевальный (12)\n", 
					u8"улучшение (9)\n", u8"фанатично (9)\n", u8"художник (8)\n", u8"цыган (5)\n", u8"честь (5)\n", 
					u8"шкаф (4)\n", u8"щёлочь (6)\n", u8"эротика (7)\n", u8"юстиция (7)\n", u8"ясно (4)\n", 
					}));
		
	}

	BOOST_TEST_CONTEXT("by ambiguity")
	{
		boost::test_tools::output_test_stream output;
		std::stringstream strings_output;
		eng.set_output(strings_output);

		BOOST_TEST_INFO("top 1 most ambiguous is the only entry exported");
		if (auto love = eng.lookup_or_add_word("love"); love->is_defined() == false)
		{
			eng.define(love);
			BOOST_TEST_CHECK(love->is_defined());
		}
		eng.export_top(dictionary_creator::ComparisonType::MostAmbiguous, 1);
		BOOST_TEST_CHECK(contains_strings(strings_output, { "love", "— (verb) feel deep affection for (someone).",
					               "— (verb) like or enjoy very much." }));

		BOOST_TEST_INFO("top 25 out of 26 are entries except that ambiguous one");
		eng.export_top(dictionary_creator::ComparisonType::LeastAmbiguous, 25);
		BOOST_TEST_CHECK(contains_strings(strings_output, { "arcbishop", "bomb", "cat", "dark", "enormous", "fly",
					"govern", "holy", "inactive", "jolly", "knight", "misery", "nap", "over", "pun",
					"question", "rubish", "satellite", "thumb", "ubiquitous", "victory", "worm", 
					"xeroradiography", "yield", "zeal" }));

		BOOST_TEST_INFO("Top 0 exports nothing");
		eng.set_output(output);
		eng.export_top(dictionary_creator::ComparisonType::MostAmbiguous, 0);
		BOOST_TEST_CHECK(output.is_empty());

		BOOST_TEST_INFO("If requested number exceeds the size, all available entries are printed");
		strings_output.str({});
		eng.set_output(strings_output);
		eng.export_top(dictionary_creator::ComparisonType::LeastAmbiguous, 1'000'000);
		BOOST_TEST_CHECK(contains_strings(strings_output, { "arcbishop", "bomb", "cat", "dark", "enormous", "fly",
					"govern", "holy", "inactive", "jolly", "knight", "love", "misery", "nap", "over",
					"pun", "question", "rubish", "satellite", "thumb", "ubiquitous", "victory", "worm", 
					"xeroradiography", "yield", "zeal" }));
	}

	BOOST_TEST_CONTEXT("by frequency")
	{
		boost::test_tools::output_test_stream output;
		std::stringstream strings_output;

		eng.lookup_or_add_word("ubiquitous")->increment_counter(4);
		eng.lookup_or_add_word("love")->increment_counter(5);
		eng.lookup_or_add_word("cat")->increment_counter();

		BOOST_TEST_INFO("top 3 most frequent words are cat, love, and ubiquitous");
		eng.set_output(strings_output);
		eng.export_top(dictionary_creator::ComparisonType::MostFrequent, 3);
		BOOST_TEST_CHECK(contains_strings(strings_output, { "cat", "love", "ubiquitous" }));
		strings_output.str({});

		BOOST_TEST_INFO("top 23 out of 26 are entries cat, love, and ubiquitous");
		eng.export_top(dictionary_creator::ComparisonType::LeastFrequent, 25);
		BOOST_TEST_CHECK(contains_strings(strings_output, { "arcbishop", "bomb", "dark", "enormous", "fly", "govern", "holy",
					"inactive", "jolly", "knight", "misery", "nap", "over", "pun", "question", "rubish", "satellite",
					"thumb", "victory", "worm", "xeroradiography", "yield", "zeal" }));

		BOOST_TEST_INFO("Top 0 exports nothing");
		eng.set_output(output);
		eng.export_top(dictionary_creator::ComparisonType::MostFrequent, 0);
		BOOST_TEST_CHECK(output.is_empty());

		BOOST_TEST_INFO("If requested number exceeds the size, all available entries are printed");
		strings_output.str({});
		eng.set_output(strings_output);
		eng.export_top(dictionary_creator::ComparisonType::LeastFrequent, 1'000'000);
		BOOST_TEST_CHECK(contains_strings(strings_output, { "arcbishop", "bomb", "cat", "dark", "enormous", "fly",
					"govern", "holy", "inactive", "jolly", "knight", "love", "misery", "nap", "over",
					"pun", "question", "rubish", "satellite", "thumb", "ubiquitous", "victory", "worm", 
					"xeroradiography", "yield", "zeal" }));
	}

	BOOST_TEST_CONTEXT("exporting subsets")
	{
		boost::test_tools::output_test_stream output;
		eng.set_output(output);

		eng.export_subset(eng.get_subset("G"));
		BOOST_TEST_CHECK(output.is_equal("govern\n"));

		auto most_l_english = [] (const dictionary_creator::Entry &a, const dictionary_creator::Entry &b)
		{
			auto aword = a.get_word();
			auto bword = b.get_word();
			auto als = std::count(aword.begin(), aword.end(), 'l');
			auto bls = std::count(bword.begin(), bword.end(), 'l');

			return als > bls;
		};
		std::stringstream strings_output;
		eng.set_output(strings_output);
		eng.export_subset(eng.get_subset(most_l_english, 4));
		BOOST_TEST_INFO("most l (2) are jolly and satellite, exact surplus words are undefined");
		BOOST_TEST_CHECK(contains_strings(strings_output, { "jolly\n", "satellite\n" }));

		rus.set_output(output);

		BOOST_TEST_INFO("empty subset means no exporting done");
		rus.export_subset(rus.get_random_words(0));
		BOOST_TEST_CHECK(output.is_empty());

		rus.export_subset(rus.get_random_words(11));
		BOOST_TEST_CHECK(!output.is_empty());
	}
}
#endif

struct serialization_cleaner
{
	serialization_cleaner()
	{
	}

	~serialization_cleaner()
	{
		std::filesystem::path file = std::string{ directoryname } + "/" + filename + ".dic";

		BOOST_TEST_CHECK(std::filesystem::remove(file));
		BOOST_TEST_CHECK(std::filesystem::remove(directoryname));
	}

	const std::string directoryname = "Saved dictionaries";
	const std::string filename = "Regress testing dictionary";
};

BOOST_FIXTURE_TEST_CASE(serializing, serialization_cleaner)
{
	BOOST_TEST_CONTEXT("checking nonexistent dictionaries")
	{
		auto available = dictionary_creator::available_dictionaries();
		BOOST_TEST_CHECK(available.size() == 0u);
	}

	BOOST_TEST_CONTEXT("saving empty")
	{
		dictionary_creator::DictionaryManager manager(dictionary_creator::Language::English, filename);
		BOOST_TEST_CHECK(manager.get_undefined().size() == 0u);

		manager.save_dictionary();
		BOOST_TEST_CHECK(dictionary_creator::available_dictionaries().size() == 1u);
	}

	BOOST_TEST_CONTEXT("loading empty and saving populated")
	{
		auto every = dictionary_creator::available_dictionaries();
		BOOST_TEST_CHECK(every.size() == 1u);

		const auto our_dictionary = std::find_if(every.begin(), every.end(), [this] (const auto &x) { return x.human_readable == filename; });
		const bool our_dictionary_valid = our_dictionary != every.end();
		BOOST_TEST_REQUIRE(our_dictionary_valid);

		auto loaded = dictionary_creator::load_dictionary(our_dictionary->full);
		BOOST_TEST_CHECK(loaded.get_undefined().size() == 0u);
		BOOST_TEST_CHECK(loaded.get_name() == filename);

		loaded.lookup_or_add_word("answer");
		loaded.lookup_or_add_word("accurate");

		loaded.save_dictionary();
		BOOST_TEST_CHECK(loaded.get_undefined().size() == 2u);
	}

	BOOST_TEST_CONTEXT("loading populated saving defined")
	{
		auto every = dictionary_creator::available_dictionaries();
		const auto our_dictionary = std::find_if(every.begin(), every.end(), [this] (const auto &x) { return x.human_readable == filename; });
		auto loaded = dictionary_creator::load_dictionary(our_dictionary->full);

		auto undefined = loaded.get_undefined();
		BOOST_TEST_CHECK(undefined.size() == 2u);
		BOOST_TEST_CHECK(loaded.contains_word("answer"));
		BOOST_TEST_CHECK(loaded.contains_word("accurate"));

		loaded.define(undefined);
		BOOST_TEST_CHECK(loaded.get_undefined().size() == 0u);

		loaded.save_dictionary();
	}
}
