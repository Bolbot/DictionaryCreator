#define BOOST_TEST_MODULE Dictionary Creator Regress Test
#include <boost/test/unit_test.hpp>

#include "dictionary_creator.h"
#include "string_examples.h"

#include <sstream>

BOOST_AUTO_TEST_SUITE(dictionary_creator)

	BOOST_AUTO_TEST_CASE(dictionary_creator_basics)
	{
		constexpr auto DictionaryCreator_minimal_possible_size =
				sizeof(Language) + sizeof(std::vector<std::unique_ptr<std::istream>>) +
				sizeof(dictionary_creator::utf8_string) + sizeof(size_t) +
				3 * sizeof(pcre_parser::RegexParser);

		dictionary_creator::DictionaryCreator english(dictionary_creator::Language::English);

		BOOST_TEST_INFO("object size");
		BOOST_TEST_CHECK(sizeof(english) >= DictionaryCreator_minimal_possible_size);
	}

	BOOST_AUTO_TEST_CASE(parsing_most_simple_pieces)
	{
		dictionary_creator::DictionaryCreator english(dictionary_creator::Language::English);

		BOOST_TEST_CONTEXT("simplest english string")
		{
			std::stringstream stream;

			auto input = std::make_unique<std::istream>(stream.rdbuf());
			english.add_input(std::move(input));

			stream << "This is an example of english string. Русские слова не считываются."
			       << "ALSO LET'S SAY This WAS SOME SORT OF NAME" << std::endl;

			auto dictionary = english.parse_to_dictionary();

			BOOST_TEST_CHECK(dictionary.lookup("example") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("english") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("string") != nullptr);
			BOOST_TEST_CHECK(dictionary.total_words() == 3u);
		}

		BOOST_TEST_CONTEXT("some texts in one stream")
		{
			std::stringstream stream;
			stream << s_ex::cpp_program << s_ex::eng_subtitles_1
				<< s_ex::eng_subtitles_2 << s_ex::eng_subtitles_3 << std::endl;

			auto input = std::make_unique<std::istream>(stream.rdbuf());

			english.add_input(std::move(input));

			auto dictionary = english.parse_to_dictionary();

			BOOST_TEST_CHECK(dictionary.lookup("include") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("myths") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("opera") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("pocket") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("rocket") == nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("antidisestablishmentarianism") == nullptr);
		}

		BOOST_TEST_CONTEXT("some texts in different streams")
		{
			std::stringstream s1, s2, s3, s4;
			s1 << s_ex::eng_subtitles_1;
			s2 << s_ex::eng_subtitles_2;
			s3 << s_ex::eng_subtitles_3;
			s4 << s_ex::cpp_program;

			english.add_input(std::make_unique<std::istream>(s1.rdbuf()));
			english.add_input(std::make_unique<std::istream>(s2.rdbuf()));
			english.add_input(std::make_unique<std::istream>(s3.rdbuf()));
			english.add_input(std::make_unique<std::istream>(s4.rdbuf()));

			auto dictionary = english.parse_to_dictionary();

			BOOST_TEST_CHECK(dictionary.lookup("include") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("myths") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("opera") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("pocket") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("rocket") == nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("antidisestablishmentarianism") == nullptr);
		}
	}

	BOOST_AUTO_TEST_CASE(dealing_with_many_inputs, * boost::unit_test::timeout(10))
	{
		dictionary_creator::DictionaryCreator english(dictionary_creator::Language::English);

		for (size_t i = 0; i != 1'000; ++i)
		{
			std::stringstream s1, s2, s3, s4, s5;
			s1 << s_ex::eng_subtitles_1;
			s2 << s_ex::eng_subtitles_2;
			s3 << s_ex::eng_subtitles_3;
			s4 << s_ex::cpp_program;
			s5 << "and also some superheroes: Superman, Batman, Spiderman, and Punisher";

			english.add_input(std::make_unique<std::istream>(s1.rdbuf()));
			english.add_input(std::make_unique<std::istream>(s2.rdbuf()));
			english.add_input(std::make_unique<std::istream>(s3.rdbuf()));
			english.add_input(std::make_unique<std::istream>(s4.rdbuf()));
			english.add_input(std::make_unique<std::istream>(s5.rdbuf()));

			auto dictionary = english.parse_to_dictionary();

			BOOST_TEST_CHECK(dictionary.lookup("include") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("myths") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("opera") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("pocket") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("rocket") == nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("antidisestablishmentarianism") == nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("superheroes") != nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("Superman") == nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("Batman") == nullptr);
			BOOST_TEST_CHECK(dictionary.lookup("Spiderman") == nullptr);
		}
	}

BOOST_AUTO_TEST_SUITE_END()
