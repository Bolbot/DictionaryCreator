#define BOOST_TEST_MODULE Test Dictionary Exporter
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#include "dictionary.h"
#include <sstream>

BOOST_AUTO_TEST_SUITE(exporter_alltogether)

	bool exports_word(const dictionary_creator::Dictionary &dictionary, const dictionary_creator::utf8_string &word);
	bool exports_words(const dictionary_creator::Dictionary &dictionary, const std::initializer_list<dictionary_creator::utf8_string> &words);

	BOOST_AUTO_TEST_CASE(exporter_alltogether)
	{
		boost::test_tools::output_test_stream stream;
		dictionary_creator::DictionaryExporter dex(&stream, "+++");

		std::stringstream sstr;
		dictionary_creator::DictionaryExporter detailed(&sstr, "000");

		BOOST_TEST_INFO("object size");
		BOOST_TEST_CHECK(sizeof(dex) >= sizeof(std::ofstream *) + sizeof(dictionary_creator::utf8_string));

		BOOST_TEST_INFO("throw upon malformed default object creation");
		BOOST_CHECK_THROW((dictionary_creator::DictionaryExporter{}), dictionary_creator::dictionary_runtime_error);

		BOOST_TEST_CONTEXT("export_dicitonary(empty)")
		{
			dictionary_creator::Dictionary empty(dictionary_creator::Language::French);
			BOOST_TEST_CHECK(empty.total_words() == 0u);

			BOOST_TEST_CHECK(&dex.export_dictionary(empty) == &stream);
			BOOST_TEST_CHECK(stream.is_empty());
		}

		dictionary_creator::Dictionary simple(dictionary_creator::Language::English);

		BOOST_TEST_CONTEXT("export_dictionary()")
		{
			simple.add_word("one");
			BOOST_TEST_CHECK(simple.total_words() == 1u);

			dex.export_dictionary(simple);
			detailed.export_dictionary(simple);

			BOOST_TEST_CHECK(!stream.is_empty());
			BOOST_TEST_CHECK(sstr.str().find("one") != std::string::npos);
		}

		sstr.str(std::string{});
		BOOST_TEST_CHECK(stream.is_empty());

		BOOST_TEST_CONTEXT("export_entries(empty)")
		{
			BOOST_TEST_CHECK(&dex.export_entries(dictionary_creator::subset_t{}) == &stream);

			BOOST_TEST_CHECK(stream.is_empty());
		}

		BOOST_TEST_CONTEXT("export_entries()")
		{
			auto nonempty_subset = simple.get_random_words(100);
			BOOST_TEST_CHECK(nonempty_subset.size() > 0u);

			dex.export_entries(nonempty_subset);
			detailed.export_entries(nonempty_subset);

			BOOST_TEST_CHECK(!stream.is_empty());
			BOOST_TEST_CHECK(sstr.str().find("one") != std::string::npos);
		}

		sstr.str(std::string{});

		BOOST_TEST_INFO("temporary object exports dictionary");
		dictionary_creator::DictionaryExporter(&sstr).export_dictionary(simple);
		BOOST_TEST_CHECK(sstr.str().find("one") != std::string::npos);

		BOOST_TEST_CONTEXT("export recently added words")
		{
			simple.add_word("knife");
			BOOST_TEST_CHECK(simple.total_words() == 2u);

			BOOST_TEST_CHECK(exports_word(simple, "knife"));
			BOOST_TEST_CHECK(exports_words(simple, { "one", "knife" }));
		}
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

BOOST_AUTO_TEST_SUITE_END()
