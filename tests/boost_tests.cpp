#define BOOST_TEST_MODULE Testing DictionaryCreator features
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#define NOMINMAX
#include <type_traits>

#include "connections.h"
#include "dictionary_definer.h"
#include "dictionary.h"
#include "dictionary_creator.h"
#include "regex_parser.h"
#include "boost_test_utilities.h"
#include "string_examples.h"

BOOST_AUTO_TEST_SUITE(Dictionary_library_testing_suite)


	BOOST_AUTO_TEST_CASE(Entry_regress_testing)
	{
		dictionary_creator::utf8_string word{ "antidisestablismentarianism" };

		dictionary_creator::Entry entry(word);

		BOOST_TEST_INFO("object size");
		BOOST_TEST_CHECK(sizeof(entry) >= DictionaryEntry_minimal_possible_size);

		BOOST_TEST_INFO("get_word()");
		BOOST_TEST_CHECK(entry.get_word() == word);

		BOOST_TEST_CONTEXT("counter - get_counter() and increment counter()")
		{
			BOOST_TEST_CHECK(entry.get_counter() == 1U);
			entry.increment_counter();
			BOOST_TEST_CHECK(entry.get_counter() == 2U);
			entry.increment_counter(1'000'000);
			BOOST_TEST_CHECK(entry.get_counter() == 1'000'002U);
		}

		BOOST_TEST_CONTEXT("define(definer_t)")
		{
			BOOST_TEST_CHECK(entry.is_defined() == false);

			dictionary_creator::definitions_t definitions = fake_definer(true);
			const auto &define_res = entry.define(fake_definer);

			BOOST_TEST_CHECK(define_res == entry);
			BOOST_TEST_CHECK(entry.get_definitions() == definitions);
		}
		
		BOOST_TEST_INFO("operator const char *()");
		BOOST_TEST_CHECK(static_cast<const char *>(entry) == word.data());

		BOOST_TEST_CONTEXT("Default uninitialized object")
		{
			dictionary_creator::Entry empty_entry;
			BOOST_TEST_CHECK(empty_entry.get_word() == "-");
		}
	}

	BOOST_AUTO_TEST_CASE(Connections_regress_testing)
	{
		const std::string html_doc_start{ "<!doctype html>" };
		const std::string html_doc_end{ "</html>" };
		const std::string http_0{ "HTTP 0 " };
		const std::string http_404{ "HTTP 404 " };

		BOOST_TEST_INFO("get for valid http address yields html document");
		auto http_connect_res = connections::get("http://www.google.com/");
		BOOST_TEST_CHECK((boost::iequals(http_connect_res.substr(0, html_doc_start.size()), html_doc_start) &&
				boost::iequals(http_connect_res.substr(http_connect_res.size() - html_doc_end.size()), html_doc_end)));

		BOOST_TEST_INFO("get for valid https address yields html document");
		auto https_connect_res = connections::get("https://dictionaryapi.dev/");
		BOOST_TEST_CHECK((boost::iequals(https_connect_res.substr(0, html_doc_start.size()), html_doc_start) &&
				boost::iequals(https_connect_res.substr(https_connect_res.size() - html_doc_end.size()), html_doc_end)));

		BOOST_TEST_INFO("get for invalid https address yields HTTP 0 response");
		auto nonexistent_res = connections::get("https://thi.s.page.does.not.exist/");
		BOOST_TEST_CHECK(nonexistent_res.substr(0, http_0.size()) == http_0);

		BOOST_TEST_INFO("get for nonexistent https page yields HTTP 404 response");
		auto notfound_res = connections::get("https://google.com/this/page/cann/ot.befound");
		BOOST_TEST_CHECK(notfound_res.substr(0, http_404.size()) == http_404);
	}

	BOOST_AUTO_TEST_CASE(Definer_regress_testing)
	{
		BOOST_TEST_CONTEXT("JSON related features")
		{
			const nlohmann::basic_json<std::map, std::vector, std::string> empty_json{};
			const auto substantial_json = parse_json(unprocessed_json_string);

			auto invalid_json_res = parse_json("{ \"invalid\" : [}");
			BOOST_TEST_CHECK(invalid_json_res == empty_json);
			
			auto trivial_json_res = parse_json("{ \"trivial\" : [ 1, 2, 4, 8 ]}");
			BOOST_TEST_CHECK(trivial_json_res != empty_json);

			auto no_definitions_json_set = json_to_definitions_set(trivial_json_res);
			BOOST_TEST_CHECK(no_definitions_json_set.empty());

			auto many_definitions_json_set = json_to_definitions_set(substantial_json);
			BOOST_TEST_CHECK(many_definitions_json_set.size() == 11);

			auto some_definitions_json_map = json_to_definitions_map(substantial_json);
			BOOST_TEST_CHECK(some_definitions_json_map.size() == 2);
		}

		BOOST_TEST_CONTEXT("IRI encoding of unicode strings")
		{
			auto naive = dictionary_creator::percent_encode("naïve");
			BOOST_TEST_CHECK(naive == R"(na%C3%AFve)");

			auto scheisse = dictionary_creator::percent_encode("SCHEIẞE");
			BOOST_TEST_CHECK(scheisse == R"(SCHEI%E1%BA%9EE)");

			auto zeppelin = dictionary_creator::percent_encode("дирижабль");
			BOOST_TEST_CHECK(zeppelin == R"(%D0%B4%D0%B8%D1%80%D0%B8%D0%B6%D0%B0%D0%B1%D0%BB%D1%8C)");
		}

		BOOST_TEST_CONTEXT("define_word()")
		{
			auto definable_eng_word = dictionary_creator::define_word("naïve", dictionary_creator::Language::English);
			BOOST_TEST_CHECK(definable_eng_word.size() > 0);

			auto undefinable_eng_word = dictionary_creator::define_word("yoziii", dictionary_creator::Language::English);
			BOOST_TEST_CHECK(undefinable_eng_word.size() == 0);

			auto definable_rus_word = dictionary_creator::define_word(u8"дирижабль", dictionary_creator::Language::Russian);
			BOOST_TEST_CHECK(definable_eng_word.size() > 0);

			auto definitions = dictionary_creator::define_word(u8"SCHEIẞE", dictionary_creator::Language::German);

			BOOST_TEST_MESSAGE(u8"define_word(\"SCHEIẞE\", de)) yields following:");
			for (const auto &[pos, defs]: definitions)
			{
				for (const auto &d: defs)
				{
					BOOST_TEST_MESSAGE('\t' << pos << " - " << d);
				}
			}

			BOOST_CHECK_THROW(define_word("hello", dictionary_creator::Language::Uninitialized), dictionary_creator::dictionary_runtime_error);
		}
	}

	struct TwoEntries
	{
		TwoEntries() : a{ new dictionary_creator::Entry("first") }, b{ new dictionary_creator::Entry("second") }
		{
			auto definer = [] (dictionary_creator::utf8_string word)
				{ return define_word(std::move(word), dictionary_creator::Language::English); };
			a->define(definer);
			a->increment_counter();
			b->define(definer);
		}

		std::shared_ptr<dictionary_creator::Entry> a;
		std::shared_ptr<dictionary_creator::Entry> b;
	};

	BOOST_FIXTURE_TEST_CASE(Sorters_regress_testing, TwoEntries, * boost::unit_test::disabled())
	{
		using dictionary_creator::criteria_dependent_sorters;
		BOOST_TEST_CONTEXT("comparing by frequency")
		{
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostFrequent)](a, b) == true);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostFrequent)](b, a) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostFrequent)](b, b) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastFrequent)](a, a) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastFrequent)](a, b) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastFrequent)](b, a) == true);
		}

		BOOST_TEST_CONTEXT("comparing by length")
		{
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Longest)](a, b) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Longest)](b, a) == true);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Longest)](a, a) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Shortest)](a, b) == true);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Shortest)](b, a) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Shortest)](a, a) == false);

			auto ca = std::make_shared<dictionary_creator::Entry>(u8"ça");
			auto va = std::make_shared<dictionary_creator::Entry>(u8"va");
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Longest)](ca, va) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Shortest)](ca, va) == false);
		}

		BOOST_TEST_CONTEXT("comparing by ambiguity")
		{
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostAmbiguous)](a, a) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastAmbiguous)](a, a) == false);
			BOOST_TEST_REQUIRE(a->is_defined());
			BOOST_TEST_REQUIRE(b->is_defined());
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostAmbiguous)](a, b) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostAmbiguous)](b, a) == true);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastAmbiguous)](a, b) == true);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastAmbiguous)](b, a) == false);
		}

		BOOST_TEST_CONTEXT("DefaultEntrySorter")
		{
			dictionary_creator::DefaultEntrySorter sorter;
			BOOST_TEST_CHECK(sorter(a, b) == true);
			BOOST_TEST_CHECK(sorter(b, a) == false);
			BOOST_TEST_CHECK(sorter(a, a) == false);

			BOOST_TEST_CHECK(sorter(a, "zoo") == true);
			BOOST_TEST_CHECK(sorter(a, "america") == false);
			BOOST_TEST_CHECK(sorter(a, "first") == false);
			BOOST_TEST_CHECK(sorter("season", b) == true);
			BOOST_TEST_CHECK(sorter("seizure", b) == false);
			BOOST_TEST_CHECK(sorter("second", b) == false);
		}
	}

	struct DictionaryObjects
	{
		DictionaryObjects() :
			eng{ dictionary_creator::Dictionary(dictionary_creator::Language::English) },
			rus{ dictionary_creator::Dictionary(dictionary_creator::Language::Russian) }
		{
			for (auto i: english_words)
			{
				eng.add_word(i);
			}

			for (auto i: russian_words)
			{
				rus.add_word(i);
			}

			BOOST_TEST_REQUIRE(eng.total_words() == english_words.size());
			BOOST_TEST_REQUIRE(rus.total_words() == russian_words.size());
		}

		dictionary_creator::Dictionary eng;
		dictionary_creator::Dictionary rus;
	};

	BOOST_AUTO_TEST_CASE(Dictionary_basic_features_regress_testing)
	{
		dictionary_creator::Dictionary object(dictionary_creator::Language::English);

		BOOST_TEST_INFO("object size");
		BOOST_TEST_CHECK(sizeof(object) >= Dictionary_minimal_possible_size);

		BOOST_TEST_INFO("get_language()");
		BOOST_TEST_CHECK((object.get_language() == dictionary_creator::Language::English));

		BOOST_TEST_INFO("total_words()");
		BOOST_TEST_CHECK(object.total_words() == 0u);

		BOOST_TEST_INFO("lookup()");
		BOOST_TEST_CHECK(object.lookup("something").get() == nullptr);

		BOOST_TEST_CONTEXT("add_word()")
		{
			BOOST_TEST_CHECK(object.add_word("something"));
			BOOST_TEST_CHECK(object.total_words() == 1u);
			BOOST_TEST_CHECK(object.lookup("something").get() != nullptr);

			BOOST_TEST_CHECK(object.add_word("something") == false);
			BOOST_TEST_CHECK(object.total_words() == 1u);
			BOOST_TEST_CHECK(object.lookup("something").get() != nullptr);
		}

		BOOST_TEST_CONTEXT("remove_word()")
		{
			BOOST_TEST_CHECK(object.remove_word("something"));
			BOOST_TEST_CHECK(object.total_words() == 0u);
			BOOST_TEST_CHECK(object.lookup("something").get() == nullptr);
		}
		
		BOOST_TEST_CONTEXT("add_proper_noun()")
		{
			object.add_word("bill");
			BOOST_TEST_CHECK(exports_word(object, "bill") == true);
			object.add_proper_noun("bill");
			BOOST_TEST_CHECK(object.total_words() == 1u);
			BOOST_TEST_CHECK(exports_word(object, "bill") == true);
			object.remove_proper_nouns();
			BOOST_TEST_REQUIRE(exports_word(object, "bill") == false);
			BOOST_TEST_CHECK(object.total_words() == 0u);
		}
	}

	BOOST_FIXTURE_TEST_CASE(Dictionary_subsets_regress_testing, DictionaryObjects)
	{
		for (auto letter: dictionary_creator::utf8_string{ "ABCDEFGhijklmnopQRSTUVWxyz" })
		{
			BOOST_TEST_INFO("get_letter_entries(" << letter << ")");
			BOOST_TEST_CHECK(eng.get_letter_entries(dictionary_creator::letter_type(1, letter)).size() == 1u);
		}

		BOOST_TEST_CONTEXT("get_undefined()")
		{
			BOOST_TEST_REQUIRE(eng.get_undefined().size() == 26u);
			eng.lookup("love")->define(fake_definer);
			BOOST_TEST_CHECK(eng.get_undefined().size() == 25u);
		}

		BOOST_TEST_CONTEXT("get_top(length)")
		{
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::Shortest, 3).size() == 3u);
			auto eng_shortest_4 = eng.get_top(dictionary_creator::ComparisonType::Shortest, 4);

			BOOST_TEST_CHECK(contains_word(eng_shortest_4, "cat"));
			BOOST_TEST_CHECK(contains_word(eng_shortest_4, "nap"));
			BOOST_TEST_CHECK(contains_word(eng_shortest_4, "fly"));
			BOOST_TEST_CHECK(contains_word(eng_shortest_4, "pun"));

			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::Longest, 1), "xeroradiography"));

			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::Shortest, 0).size() == 0u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::Longest, 0).size() == 0u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::Shortest, 1'000'000).size() == 26u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::Longest, 1'000'000).size() == 26u);
		}

		BOOST_TEST_CONTEXT("get_top(ambiguity)")
		{
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::MostAmbiguous, 999).size() == 26u);
			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::MostAmbiguous, 1), "love"));
			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::LeastAmbiguous, 25), "love") == false);

			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::MostAmbiguous, 0).size() == 0u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::MostAmbiguous, 1'000'000).size() == 26u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::LeastAmbiguous, 0).size() == 0u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::LeastAmbiguous, 1'000'000).size() == 26u);
		}

		BOOST_TEST_CONTEXT("get_top(frequency)")
		{
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::MostFrequent, 1).size() == 1u);
			eng.lookup("ubiquitous")->increment_counter();
			eng.lookup("love")->increment_counter();
			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::MostFrequent, 7), "ubiquitous") == true);
			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::MostFrequent, 7), "love") == true);
			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::LeastFrequent, 7), "ubiquitous") == false);
			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::LeastFrequent, 7), "love") == false);

			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::MostFrequent, 0).size() == 0u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::MostFrequent, 1'000'000).size() == 26u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::LeastFrequent, 0).size() == 0u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::LeastFrequent, 1'000'000).size() == 26u);
		}

		BOOST_TEST_CONTEXT("get_random_word() and get_random_words()")
		{
			BOOST_TEST_CHECK(eng.get_random_word().get() != nullptr);
			BOOST_TEST_CHECK(eng.get_random_words(11).size() == 11u);
			BOOST_TEST_CHECK(eng.get_random_words(0).size() == 0u);
			BOOST_TEST_CHECK(eng.get_random_words(1'000'000).size() == 26u);
		}
	}

	BOOST_FIXTURE_TEST_CASE(Dictionary_custom_types_regress_testing, DictionaryObjects)
	{
		BOOST_TEST_CONTEXT("add_word<CustomType>()")
		{
			BOOST_TEST_CHECK(eng.add_word<DifficultyEntry>("red", 0.1));
			eng.add_word<DifficultyEntry>("green", 0.15);
			eng.add_word<DifficultyEntry>("blue", 0.12);
			eng.add_word<DifficultyEntry>("antidisestablishmentarianism", 3.5);
			BOOST_TEST_CHECK(eng.total_words() == 30u);

			BOOST_TEST_INFO("previously present word");
			eng.add_word<DifficultyEntry>("satellite", 1.2);
			BOOST_TEST_CHECK(eng.total_words() == 30u);
		}

		BOOST_TEST_CONTEXT("get_top(least difficult)")
		{
			BOOST_TEST_CHECK(contains_word(eng.get_top(get_comp_less_difficult(), 3), "red"));
			BOOST_TEST_CHECK(contains_word(eng.get_top(get_comp_less_difficult(), 3), "green"));
			BOOST_TEST_CHECK(contains_word(eng.get_top(get_comp_less_difficult(), 3), "blue"));
			BOOST_TEST_CHECK(contains_word(eng.get_top(get_comp_less_difficult(), 3), "antidisestablishmentarianism") == false);
			BOOST_TEST_CHECK(eng.get_top(get_comp_less_difficult(), 3).front()->get_word() == "red");
		}

		BOOST_TEST_CONTEXT("get_top(most difficult)")
		{
			BOOST_TEST_CHECK(contains_word(eng.get_top(get_comp_more_difficult(), 4), "red") == false);
			BOOST_TEST_CHECK(contains_word(eng.get_top(get_comp_more_difficult(), 4), "green"));
			BOOST_TEST_CHECK(contains_word(eng.get_top(get_comp_more_difficult(), 4), "blue"));
			BOOST_TEST_CHECK(contains_word(eng.get_top(get_comp_more_difficult(), 4), "satellite"));
			BOOST_TEST_CHECK(contains_word(eng.get_top(get_comp_more_difficult(), 4), "antidisestablishmentarianism"));
			BOOST_TEST_CHECK(eng.get_top(get_comp_more_difficult(), 4).front()->get_word() == "antidisestablishmentarianism");
		}

		BOOST_TEST_CONTEXT("get_top() quantity of subtype pointers")
		{
			BOOST_TEST_CHECK(eng.get_top(get_comp_less_difficult(), 5).back().get() != nullptr);
			BOOST_TEST_CHECK(dynamic_cast<DifficultyEntry *>(eng.get_top(get_comp_less_difficult(), 6).front().get()) != nullptr);
			BOOST_TEST_CHECK(dynamic_cast<DifficultyEntry *>(eng.get_top(get_comp_less_difficult(), 6).back().get()) == nullptr);
			eng.add_word<DifficultyEntry>("pun", 0.1);
			BOOST_TEST_CHECK(dynamic_cast<DifficultyEntry *>(eng.get_top(get_comp_less_difficult(), 6).back().get()) != nullptr);
		}
	}

	BOOST_FIXTURE_TEST_CASE(Dictionary_big_five_regress_testing, DictionaryObjects)
	{
		auto get_english = [this] () -> dictionary_creator::Dictionary { auto res = eng; return std::move(res); };

		eng.add_proper_noun("Anna");
		BOOST_TEST_CHECK(eng.total_words() == 26u);

		BOOST_TEST_CONTEXT("copy construction")
		{
			dictionary_creator::Dictionary object = eng;
			BOOST_TEST_CHECK(exports_words(object, english_words));

			object.add_word("Anna");
			BOOST_TEST_CHECK(object.total_words() == 26u + 1u);

			object.remove_proper_nouns();
			BOOST_TEST_CHECK(object.total_words() == 26u);
		}

		BOOST_TEST_CONTEXT("move construction")
		{
			dictionary_creator::Dictionary object = get_english();
			BOOST_TEST_CHECK(exports_words(object, english_words));

			object.add_word("Anna");
			BOOST_TEST_CHECK(exports_word(object, "Anna") == true);

			object.remove_proper_nouns();
			BOOST_TEST_CHECK(object.total_words() == 26u);
			BOOST_TEST_CHECK(exports_word(object, "Anna") == false);
		}

		rus.add_word(u8"Василий");
		rus.add_proper_noun(u8"Василий");
		dictionary_creator::Dictionary object = eng;

		BOOST_TEST_CONTEXT("copy assignment")
		{
			object = rus;
			BOOST_TEST_CHECK((object.get_language() == dictionary_creator::Language::Russian));
			BOOST_TEST_CHECK(exports_words(object, russian_words));
			BOOST_TEST_CHECK(exports_word(object, u8"Василий") == true);

			object.remove_proper_nouns();
			BOOST_TEST_CHECK(exports_word(object, u8"Василий") == false);
		}

		BOOST_TEST_CONTEXT("move assignment")
		{
			dictionary_creator::Dictionary dest(dictionary_creator::Language::French);
			object.add_proper_noun(u8"Япония");

			dest = std::move(object);
			BOOST_TEST_CHECK((dest.get_language() == dictionary_creator::Language::Russian));
			BOOST_TEST_CHECK(exports_word(dest, u8"Василий") == false);
			BOOST_TEST_CHECK(exports_word(dest, u8"Япония") == false);

			dest.add_word(u8"Япония");
			BOOST_TEST_CHECK(exports_word(dest, u8"Япония") == true);
			BOOST_TEST_CHECK(exports_words(dest, russian_words));

			dest.remove_proper_nouns();
			BOOST_TEST_CHECK(exports_word(dest, u8"Япония") == false);
			BOOST_TEST_CHECK(exports_words(dest, russian_words));
		}

		BOOST_TEST_CONTEXT("self assignment")
		{
			BOOST_TEST_CHECK(exports_words(rus, russian_words) == true);
			BOOST_TEST_CHECK(exports_word(rus, u8"Василий") == true);

			rus = rus;

			BOOST_TEST_CHECK(exports_words(rus, russian_words) == true);
			BOOST_TEST_CHECK(exports_word(rus, u8"Василий") == true);

			rus.remove_proper_nouns();

			BOOST_TEST_CHECK(exports_words(rus, russian_words) == true);
			BOOST_TEST_CHECK(exports_word(rus, u8"Василий") == false);

			rus.add_word(u8"Екатерина");
			rus.add_proper_noun(u8"Екатерина");

			BOOST_TEST_CHECK(exports_word(rus, u8"Екатерина") == true);

			BOOST_TEST_MESSAGE("before self assignment Dictionary object contains\t" << rus.total_words() << " words");
			rus = std::move(rus);
			BOOST_TEST_MESSAGE("after self assignment Dictionary object contains\t" << rus.total_words() << " words");

			rus = eng;
			BOOST_TEST_CHECK(exports_words(rus, english_words) == true);
			BOOST_TEST_CHECK(exports_word(rus, u8"Василий") == false);
			BOOST_TEST_CHECK(exports_word(rus, u8"Екатерина") == false);

			rus.add_word(u8"Василий");
			rus.add_word(u8"Екатерина");
			rus.remove_proper_nouns();
			
			BOOST_TEST_CHECK(exports_word(rus, u8"Василий") == true);
			BOOST_TEST_CHECK(exports_word(rus, u8"Екатерина") == true);
		}
	}

	struct DictionaryObjectsLight
	{
		dictionary_creator::Dictionary abcd;
		dictionary_creator::Dictionary efgh;
		dictionary_creator::Dictionary ijkl;

		dictionary_creator::Dictionary eng;
		dictionary_creator::Dictionary rus;

		DictionaryObjectsLight() : abcd{ dictionary_creator::Language::English }, efgh{ dictionary_creator::Language::English },
			ijkl{ dictionary_creator::Language::English }, eng{ dictionary_creator::Language::English },
			rus{ dictionary_creator::Language::Russian }
		{
			abcd.add_word("a");
			abcd.add_word("b");
			abcd.add_word("c");
			abcd.add_word("d");

			efgh.add_word("e");
			efgh.add_word("f");
			efgh.add_word("g");
			efgh.add_word("h");

			ijkl.add_word("i");
			ijkl.add_word("j");
			ijkl.add_word("k");
			ijkl.add_word("l");
		}

		dictionary_creator::Dictionary get_rus() const
		{
			auto also_rus = rus;
			return std::move(also_rus);
		}

		bool contains(const dictionary_creator::Dictionary &dictionary, const dictionary_creator::utf8_string &words) const
		{
			for (auto i: words)
			{
				if (dictionary.lookup(dictionary_creator::utf8_string(1, i)).get() == nullptr)
				{
					return false;
				}
			}
			
			return true;
		}

		bool contains_none(const dictionary_creator::Dictionary &dictionary, const dictionary_creator::utf8_string &words) const
		{
			for (auto i: words)
			{
				if (dictionary.lookup(dictionary_creator::utf8_string(1, i)).get() != nullptr)
				{
					return false;
				}
			}
			
			return true;
		}
	};

	BOOST_FIXTURE_TEST_CASE(Dictionary_operations_regress_testing, DictionaryObjectsLight)
	{
		BOOST_TEST_INFO("throw upon attempt of mismathing languages merge");
		BOOST_CHECK_THROW(eng.merge(rus), dictionary_creator::dictionary_runtime_error);
		BOOST_CHECK_THROW(eng.merge(get_rus()), dictionary_creator::dictionary_runtime_error);

		BOOST_TEST_CONTEXT("merge(lvalue)")
		{
			auto abcd_ = abcd;
			auto efgh_ = efgh;

			BOOST_TEST_CHECK(contains(abcd_, "abcd"));
			BOOST_TEST_CHECK(contains(efgh_, "efgh"));
			BOOST_TEST_CHECK(contains_none(efgh_, "abcd"));

			abcd_.add_proper_noun("c");
			efgh_.add_proper_noun("d");
			abcd_.add_proper_noun("e");
			efgh_.add_proper_noun("f");

			abcd_.merge(efgh_);

			BOOST_TEST_CHECK(contains(abcd_, "abgh"));
			BOOST_TEST_CHECK(contains_none(abcd_, "cdef"));
			BOOST_TEST_CHECK(contains(efgh_, "efgh"));
			BOOST_TEST_CHECK(contains_none(efgh_, "abcd"));
		}
		
		BOOST_TEST_CONTEXT("merge(rvalue)")
		{
			auto object = abcd;
			BOOST_TEST_CHECK(contains(object, "abcd"));
			BOOST_TEST_CHECK(contains(efgh, "efgh"));
			BOOST_TEST_CHECK(contains_none(efgh, "abcd"));

			object.add_proper_noun("c");
			object.add_proper_noun("e");

			object.merge([this]
					{
						auto res = efgh;
						res.add_proper_noun("d");
						res.add_proper_noun("f");
						return std::move(res);
					}());

			BOOST_TEST_CHECK(contains(object, "abgh"));
			BOOST_TEST_CHECK(contains_none(object, "cdef"));
		}

		BOOST_TEST_INFO("merge(rvalue).merge(lvalue)");
		auto abcdefghijkl = abcd;
		abcdefghijkl.merge([this] { auto res = efgh; return std::move(res); }()).merge(ijkl);
		BOOST_TEST_CHECK(contains(abcdefghijkl, "abcdefghijkl"));

		BOOST_TEST_CONTEXT("self merge")
		{
			auto selfmerge = abcd;
			selfmerge.add_proper_noun("a");

			selfmerge.merge(selfmerge);

			BOOST_TEST_CHECK(contains(selfmerge, "bcd"));
			BOOST_TEST_CHECK(selfmerge.lookup("a").get() == nullptr);
		}

		BOOST_TEST_INFO("throw upon attempt of mismathing languages subtract");
		BOOST_CHECK_THROW(eng.subtract(rus), dictionary_creator::dictionary_runtime_error);
		BOOST_CHECK_THROW(eng.subtract(get_rus()), dictionary_creator::dictionary_runtime_error);

		BOOST_TEST_CONTEXT("subtract(lvalue)")
		{
			auto abcde = abcd;
			abcde.add_word("e");
			abcde.add_proper_noun("g");

			auto defgh = efgh;
			defgh.add_word("d");
			defgh.add_proper_noun("c");

			abcde.subtract(defgh);

			BOOST_TEST_CHECK(contains(abcde, "ab"));
			BOOST_TEST_CHECK(contains_none(abcde, "cdefgh"));
			BOOST_TEST_CHECK(contains(defgh, "defgh"));
			BOOST_TEST_CHECK(contains_none(defgh, "abc"));
		}

		BOOST_TEST_CONTEXT("subtract(rvalue)")
		{
			auto abcde = abcd;
			abcde.add_word("e");
			abcde.add_proper_noun("g");

			abcde.subtract([this] 
					{
						auto res = efgh;
						res.add_word("d");
						res.add_proper_noun("c");
						return std::move(res);
					}());

			BOOST_TEST_CHECK(contains(abcde, "ab"));
			BOOST_TEST_CHECK(contains_none(abcde, "cdefgh"));
		}

		BOOST_TEST_CONTEXT("compound subtract")
		{
			auto ijkl_ = abcdefghijkl;

			ijkl_.subtract(abcd).subtract([this] { auto res = efgh; return std::move(res); }());

			BOOST_TEST_CHECK(contains(ijkl_, "ijkl"));
			BOOST_TEST_CHECK(contains_none(ijkl_, "abcdefgh"));
		}

		BOOST_TEST_CONTEXT("self subtract")
		{
			auto selfsubtract = abcd;
			selfsubtract.add_proper_noun("e");
			selfsubtract.add_proper_noun("f");
			selfsubtract.add_proper_noun("g");

			selfsubtract.subtract(selfsubtract);

			BOOST_TEST_CHECK(selfsubtract.total_words() == 0u);
			selfsubtract.merge(efgh);
			BOOST_TEST_CHECK(contains(selfsubtract, "h"));
			BOOST_TEST_CHECK(selfsubtract.total_words() == 1u);
		}

		BOOST_TEST_INFO("throw upon attempt of mismathing languages intersection");
		BOOST_CHECK_THROW(eng.intersection_with(rus), dictionary_creator::dictionary_runtime_error);
		BOOST_CHECK_THROW(eng.intersection_with(get_rus()), dictionary_creator::dictionary_runtime_error);

		BOOST_TEST_CONTEXT("intersection_with(lvalue)")
		{
			auto abcdef = abcd;
			abcdef.add_word("e");
			abcdef.add_word("f");
			abcdef.add_proper_noun("c");

			auto efghi = efgh;
			efghi.add_word("i");
			efghi.add_proper_noun("f");

			BOOST_TEST_CHECK(contains(abcdef.intersection_with(efghi), "e"));
			BOOST_TEST_CHECK(contains_none(abcdef.intersection_with(efghi), "abcdfghi"));
			BOOST_TEST_CHECK(contains(efghi.intersection_with(abcdef), "e"));
			BOOST_TEST_CHECK(contains_none(efghi.intersection_with(abcdef), "abcdfghi"));
		}

		BOOST_TEST_CONTEXT("intersection_with(lvalue).intersecion_with(rvalue)")
		{
			auto abcdef = abcd;
			abcdef.add_word("e");
			abcdef.add_word("f");

			auto cdefgh = efgh;
			cdefgh.add_word("c");
			cdefgh.add_word("d");

			auto bcde = abcd;
			bcde.remove_word("a");
			bcde.add_word("e");

			auto cde = abcdef.intersection_with(cdefgh).intersection_with([&bcde]
						{ auto res = bcde; return std::move(res); }());

			BOOST_TEST_CHECK(contains(cde, "cde"));
			BOOST_TEST_CHECK(contains_none(cde, "abfghijkl"));
		}

		BOOST_TEST_CONTEXT("self intersection")
		{
			auto selfintersect = abcd;
			selfintersect.add_proper_noun("a");
			selfintersect.add_proper_noun("h");

			auto selfintersect_result = selfintersect.intersection_with(selfintersect);

			BOOST_TEST_CHECK(contains(selfintersect, "abcd"));
			BOOST_TEST_CHECK(contains(selfintersect_result, "bcd"));
			BOOST_TEST_CHECK(selfintersect_result.total_words() == 3u);

			selfintersect_result.add_word("h");
			selfintersect_result.add_word("i");
			BOOST_TEST_CHECK(selfintersect_result.total_words() == 5u);

			selfintersect_result.remove_proper_nouns();
			BOOST_TEST_CHECK(contains(selfintersect_result, "bcdi"));
			BOOST_TEST_CHECK(contains_none(selfintersect_result, "aefgh"));
		}
	}

	BOOST_FIXTURE_TEST_CASE(Dictionary_arithmetics_regress_testing, DictionaryObjectsLight)
	{
		BOOST_TEST_INFO("throw upon attempt of mismathing languages +=");
		BOOST_CHECK_THROW(eng += rus, dictionary_creator::dictionary_runtime_error);
		BOOST_CHECK_THROW(eng += get_rus(), dictionary_creator::dictionary_runtime_error);

		BOOST_TEST_CONTEXT("+= lvalue")
		{
			auto abcd_ = abcd;
			auto efgh_ = efgh;
			BOOST_TEST_CHECK(contains(abcd_, "abcd"));
			BOOST_TEST_CHECK(contains(efgh_, "efgh"));
			BOOST_TEST_CHECK(contains_none(efgh_, "abcd"));

			abcd_.add_proper_noun("c");
			efgh_.add_proper_noun("d");
			abcd_.add_proper_noun("e");
			efgh_.add_proper_noun("f");

			abcd_ += efgh_;
			BOOST_TEST_CHECK(contains(abcd_, "abgh"));
			BOOST_TEST_CHECK(contains_none(abcd_, "cdef"));
			BOOST_TEST_CHECK(contains(efgh_, "efgh"));
			BOOST_TEST_CHECK(contains_none(efgh_, "abcd"));
		}
		
		BOOST_TEST_CONTEXT("+= rvalue")
		{
			auto object = abcd;
			BOOST_TEST_CHECK(contains(object, "abcd"));
			BOOST_TEST_CHECK(contains(efgh, "efgh"));
			BOOST_TEST_CHECK(contains_none(efgh, "abcd"));

			object.add_proper_noun("c");
			object.add_proper_noun("e");

			object += [this]
				{
					auto res = efgh;
					res.add_proper_noun("d");
					res.add_proper_noun("f");
					return std::move(res);
				}();
			BOOST_TEST_CHECK(contains(object, "abgh"));
			BOOST_TEST_CHECK(contains_none(object, "cdef"));
		}

		BOOST_TEST_INFO("+= rvalue += lvalue");
		auto abcdefghijkl = abcd;
		abcdefghijkl += [this] { auto res = efgh; return std::move(res); }() += ijkl;
		BOOST_TEST_CHECK(contains(abcdefghijkl, "abcdefghijkl"));

		BOOST_TEST_CONTEXT("self +=")
		{
			auto selfmerge = abcd;
			selfmerge.add_proper_noun("a");
			selfmerge += selfmerge;
			BOOST_TEST_CHECK(contains(selfmerge, "bcd"));
			BOOST_TEST_CHECK(selfmerge.lookup("a").get() == nullptr);
		}

		BOOST_TEST_INFO("throw upon attempt of mismathing languages -=");
		BOOST_CHECK_THROW(eng -= rus, dictionary_creator::dictionary_runtime_error);
		BOOST_CHECK_THROW(eng -= get_rus(), dictionary_creator::dictionary_runtime_error);

		BOOST_TEST_CONTEXT("-= lvalue")
		{
			auto abcde = abcd;
			abcde.add_word("e");
			abcde.add_proper_noun("g");

			auto defgh = efgh;
			defgh.add_word("d");
			defgh.add_proper_noun("c");

			abcde -= defgh;

			BOOST_TEST_CHECK(contains(abcde, "ab"));
			BOOST_TEST_CHECK(contains_none(abcde, "cdefgh"));
			BOOST_TEST_CHECK(contains(defgh, "defgh"));
			BOOST_TEST_CHECK(contains_none(defgh, "abc"));
		}

		BOOST_TEST_CONTEXT("-= rvalue")
		{
			auto abcde = abcd;
			abcde.add_word("e");
			abcde.add_proper_noun("g");
			abcde -= [this]
				{
					auto res = efgh;
					res.add_word("d");
					res.add_proper_noun("c");
					return std::move(res);
				}();

			BOOST_TEST_CHECK(contains(abcde, "ab"));
			BOOST_TEST_CHECK(contains_none(abcde, "cdefgh"));
		}

		BOOST_TEST_CONTEXT("-= rvalue += lvalue")
		{
			auto object = abcdefghijkl;
			object -= [this] { auto res = efgh; return std::move(res); }() += abcd;
			BOOST_TEST_CHECK(contains(object, "ijkl"));
			BOOST_TEST_CHECK(contains_none(object, "abcdefgh"));
		}

		BOOST_TEST_CONTEXT("self -=")
		{
			auto selfsubtract = abcd;
			selfsubtract.add_proper_noun("e");
			selfsubtract.add_proper_noun("f");
			selfsubtract.add_proper_noun("g");

			selfsubtract -= selfsubtract;

			BOOST_TEST_CHECK(selfsubtract.total_words() == 0u);
			selfsubtract += efgh;
			BOOST_TEST_CHECK(contains(selfsubtract, "h"));
			BOOST_TEST_CHECK(selfsubtract.total_words() == 1u);
		}

		BOOST_TEST_INFO("throw upon attempt of mismathing languages *=");
		BOOST_CHECK_THROW(eng *= rus, dictionary_creator::dictionary_runtime_error);
		BOOST_CHECK_THROW(eng *= get_rus(), dictionary_creator::dictionary_runtime_error);

		BOOST_TEST_CONTEXT("*= lval")
		{
			auto abcdef = abcd;
			abcdef.add_word("e");
			abcdef.add_word("f");
			abcdef.add_proper_noun("c");

			auto efghi = efgh;
			efghi.add_word("i");
			efghi.add_proper_noun("f");

			BOOST_TEST_CHECK(contains([&abcdef, &efghi] { auto res = abcdef; res *= efghi; return res; } (), "e"));
			BOOST_TEST_CHECK(contains_none([&abcdef, &efghi] { auto res = abcdef; res *= efghi; return res; } (), "abcdfghi"));
			BOOST_TEST_CHECK(contains([&efghi, &abcdef] { auto res = efghi; res *= abcdef; return res; } (), "e"));
			BOOST_TEST_CHECK(contains_none([&efghi, &abcdef] { auto res = efghi; res *= abcdef; return res; } (), "abcdfghi"));
		}

		BOOST_TEST_CONTEXT("*= lval *= rval")
		{
			auto abcdef = abcd;
			abcdef.add_word("e");
			abcdef.add_word("f");

			auto cdefgh = efgh;
			cdefgh.add_word("c");
			cdefgh.add_word("d");

			auto bcde = abcd;
			bcde.remove_word("a");
			bcde.add_word("e");

			auto cde = abcdef;
			cde *= cdefgh *= [&bcde] { auto res = bcde; return std::move(res); } ();
			BOOST_TEST_CHECK(contains(cde, "cde"));
			BOOST_TEST_CHECK(contains_none(cde, "abfghijkl"));
		}

		BOOST_TEST_CONTEXT("*= self")
		{
			auto selfintersect = abcd;
			selfintersect.add_proper_noun("a");
			selfintersect.add_proper_noun("h");

			selfintersect *= selfintersect;

			BOOST_TEST_CHECK(contains(selfintersect, "bcd"));
			BOOST_TEST_CHECK(selfintersect.lookup("a").get() == nullptr);

			selfintersect.add_word("h");
			selfintersect.add_word("i");
			BOOST_TEST_CHECK(selfintersect.total_words() == 5u);

			selfintersect.remove_proper_nouns();
			BOOST_TEST_CHECK(selfintersect.lookup("h").get() == nullptr);
			BOOST_TEST_CHECK(contains(selfintersect, "bcdi"));
		}
	}

	BOOST_AUTO_TEST_CASE(DictionaryExporter_regress_testing)
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

	BOOST_AUTO_TEST_CASE(RegexParser_regress_testing)
	{
		pcre_parser::RegexParser object("");
		BOOST_TEST_INFO("object size");
		BOOST_TEST_CHECK(sizeof(object) >= RegexParser_minimal_possible_size);

		BOOST_TEST_INFO("throw upon malformed regex argument construction");
		BOOST_CHECK_THROW(pcre_parser::RegexParser("[invalid"), std::runtime_error);

		BOOST_TEST_CONTEXT("searching numbers")
		{
			pcre_parser::RegexParser numbers(R"(\b[0-9]+\b)");

			auto empty = numbers.all_matches("there are no numbers, empty multiset shall be returned");

			BOOST_TEST_CHECK(empty.empty());

			auto n123 = numbers.all_matches("Only 123 numbers n0t numb3rs w1th d1g1ts 234no, neither654.\n123, or 99, or 100!");

			BOOST_TEST_CHECK(n123.size() == 4u);
			BOOST_TEST_CHECK(n123.count("123") == 2u);
			BOOST_TEST_CHECK(n123.count("99") == 1u);
			BOOST_TEST_CHECK(n123.count("100") == 1u);
		}

		BOOST_TEST_CONTEXT("english lowercase words")
		{
			pcre_parser::RegexParser words(R"(\b[a-z]+\b)");

			auto empty = words.all_matches("193 <> 848 @@@ 4859 th1s d0es n0+ c0uN+ NOR UPPERCASE d03s");

			BOOST_TEST_CHECK(empty.empty());

			auto n123 = words.all_matches("Only 123 numbers n0t numb3rs w1th d1g1ts 234no, neither654.\n123, or 99, or 100!");

			BOOST_TEST_CHECK(n123.size() == 3u);
			BOOST_TEST_CHECK(n123.count("numbers") == 1u);
			BOOST_TEST_CHECK(n123.count("or") == 2u);

			auto cpp = words.all_matches(s_ex::cpp_program);
			BOOST_TEST_CHECK(cpp.count("include") == 6u);
			BOOST_TEST_CHECK(cpp.count("std") == 2u);
			BOOST_TEST_CHECK(cpp.size() == 34u);

			auto eng_sub_1 = words.all_matches(s_ex::eng_subtitles_1);
			BOOST_TEST_CHECK(eng_sub_1.size() == 20u);

			auto eng_sub_2 = words.all_matches(s_ex::eng_subtitles_2);
			BOOST_TEST_CHECK(eng_sub_2.size() == 19u);

			auto eng_sub_3 = words.all_matches(s_ex::eng_subtitles_3);
			BOOST_TEST_CHECK(eng_sub_3.size() == 31u);

			auto rom_sub = words.all_matches(s_ex::rom_subtitles);
			BOOST_TEST_CHECK(rom_sub.size() == 28u);
		}

		BOOST_TEST_CONTEXT("romanian words")
		{
			pcre_parser::RegexParser words(u8R"((*UTF8)\b[A-ZĂÂÎȘŞȚŢ]?[a-zăâîșşțţ]+\b)");

			auto empty = words.all_matches("193 <> 848 @@@ 4859 th1s d0es n0+ c0uN+ NOR UPPERCASE d03s");

			BOOST_TEST_CHECK(empty.empty());

			auto subtitles = words.all_matches(s_ex::rom_subtitles);
			BOOST_TEST_CHECK(subtitles.size() == 29u);
		}

		BOOST_TEST_CONTEXT("russian words")
		{

			pcre_parser::RegexParser wtf(u8R"((*UTF8)\b[вхуй]+\b)");
			auto wtf_res = wtf.all_matches(u8"грохочет хуй сверкает пизда в ночи");
			BOOST_TEST_REQUIRE(wtf_res.size() == 2u);
			for (auto i: wtf_res)
			{
				std::cout << '\t' << i << '\n';
			}

/*
			pcre_parser::RegexParser every(u8R"((*UTF8)\b[А-ЯЁ]?[а-яё]+\b)");
			pcre_parser::RegexParser lowercase(u8R"((*UTF8)\b[а-яё]+\b)");

			auto no_lowercase = every.all_matches(u8R"(Нет НИ ОДНОГО сЛОВа В НиЖнЕм Регистре!)");
			BOOST_TEST_CHECK(no_lowercase.size() == 3u);

			auto empty = lowercase.all_matches(u8R"(Нет НИ ОДНОГО сЛОВа В НиЖнЕм Регистре!)");
			BOOST_TEST_CHECK(empty.empty());

			BOOST_TEST_CHECK(every.all_matches(u8R"(КАПС НЕ СЧИТАЕТСЯ тЕМ бОлЕе тАк 111 !!!)").empty());

			auto rus_sub_1_low = lowercase.all_matches(s_ex::rus_subtitles_1);
			BOOST_TEST_CHECK(rus_sub_1_low.size() == 10u);

			auto rus_sub_1_all = every.all_matches(s_ex::rus_subtitles_1);
			BOOST_TEST_CHECK(rus_sub_1_all.size() == 17u);

			auto rus_sub_2_low = lowercase.all_matches(s_ex::rus_subtitles_2);
			BOOST_TEST_CHECK(rus_sub_2_low.size() == 18u);

			auto rus_sub_2_all = every.all_matches(s_ex::rus_subtitles_2);
			BOOST_TEST_CHECK(rus_sub_2_all.size() == 25u);
*/			
		}
	}

	BOOST_AUTO_TEST_CASE(DictionaryCreator_regress_testing)
	{
		dictionary_creator::DictionaryCreator dic(dictionary_creator::Language::English);
		BOOST_TEST_INFO("object size");
		BOOST_TEST_CHECK(sizeof(dic) >= DictionaryCreator_minimal_possible_size);
	}


BOOST_AUTO_TEST_SUITE_END()
