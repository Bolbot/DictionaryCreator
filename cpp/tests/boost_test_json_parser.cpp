#define BOOST_TEST_MODULE Json Parser Regress Test
#include <boost/test/unit_test.hpp>

#include "json_parser.h"
#include "json_parser_impl.h"

#include <vector>

BOOST_AUTO_TEST_SUITE(json_parser_testing)

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

BOOST_AUTO_TEST_SUITE_END()
