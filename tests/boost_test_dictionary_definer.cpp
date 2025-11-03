#define BOOST_TEST_MODULE Dictionary Definer Regress Test
#include <boost/test/unit_test.hpp>

#include "dictionary_definer.h"
#include "json_parser.h"

BOOST_AUTO_TEST_SUITE(definer_alltogether)

BOOST_AUTO_TEST_CASE(IRI_encoding)
{
	auto naive = dictionary_creator::percent_encode("naïve");
	BOOST_TEST_CHECK(naive == R"(na%C3%AFve)");

	auto scheisse = dictionary_creator::percent_encode("SCHEIẞE");
	BOOST_TEST_CHECK(scheisse == R"(SCHEI%E1%BA%9EE)");

	auto zeppelin = dictionary_creator::percent_encode("дирижабль");
	BOOST_TEST_CHECK(zeppelin == R"(%D0%B4%D0%B8%D1%80%D0%B8%D0%B6%D0%B0%D0%B1%D0%BB%D1%8C)");
}

BOOST_AUTO_TEST_CASE(define_word)
{
	auto definable_eng_word = dictionary_creator::define_word("naïve", dictionary_creator::Language::English);
	BOOST_TEST_CHECK(definable_eng_word.size() > 0u);

	auto undefinable_eng_word = dictionary_creator::define_word("yoziii", dictionary_creator::Language::English);
	BOOST_TEST_CHECK(undefinable_eng_word.size() == 0u);

	auto definable_rus_word = dictionary_creator::define_word(u8"дирижабль", dictionary_creator::Language::Russian);
	BOOST_TEST_CHECK(definable_eng_word.size() > 0u);

	auto definitions = dictionary_creator::define_word(u8"SCHEIẞE", dictionary_creator::Language::German);

	BOOST_TEST_MESSAGE(u8"define_word(\"SCHEIẞE\", de)) yields following:");
	for (const auto &[pos, defs]: definitions)
	{
		for (const auto &d: defs)
		{
			BOOST_TEST_MESSAGE('\t' << pos << " - " << d);
		}
	}

	BOOST_CHECK_THROW(dictionary_creator::define_word("hello", dictionary_creator::Language::Uninitialized), dictionary_creator::dictionary_runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
