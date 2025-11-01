#define BOOST_TEST_MODULE Dictionary Definer Regress Test
#include <boost/test/unit_test.hpp>

#include "dictionary_definer.h"
#include "json_parser.h"
#include "json_parser_impl.h"

BOOST_AUTO_TEST_SUITE(definer_alltogether)

const auto unprocessed_json_string = R"([{"word":"frame","meanings": [ {"partOfSpeech":"verb","definitions":[ {"definition":"Place in a frame."}, {"definition":"Create or formulate"}, {"definition":"Produce false evidence"}, {"definition":"Erect the building."}]}, {"partOfSpeech":"noun","definitions":[ {"definition":"Structure that surrounds"}, {"definition":"A person's body"}, {"definition":"A basic structure"}, {"definition":"A structural environment"}, {"definition":"A single complete picture"}, {"definition":"A round of play in game."}, {"definition":"short for frame-up"}] }] }])";

BOOST_AUTO_TEST_CASE(json_features)
{
	const nlohmann::basic_json<std::map, std::vector, std::string> empty_json{};
	const auto substantial_json = parse_json(unprocessed_json_string);

	BOOST_TEST_CONTEXT("invalid and trivial")
	{
		auto invalid_json_res = parse_json("{ \"invalid\" : [}");
		BOOST_TEST_CHECK(invalid_json_res == empty_json);

		auto trivial_json_res = parse_json("{ \"trivial\" : [ 1, 2, 4, 8 ]}");
		BOOST_TEST_CHECK(trivial_json_res != empty_json);

		auto no_definitions_json_set = json_to_definitions_set(trivial_json_res);
		BOOST_TEST_CHECK(no_definitions_json_set.empty());
	}

	BOOST_TEST_CONTEXT("substantial")
	{
		auto many_definitions_json_set = json_to_definitions_set(substantial_json);
		BOOST_TEST_CHECK(many_definitions_json_set.size() == 11u);

		auto some_definitions_json_map = json_to_definitions_map(substantial_json);
		BOOST_TEST_CHECK(some_definitions_json_map.size() == 2u);
	}
}

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
