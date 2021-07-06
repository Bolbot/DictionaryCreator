#define BOOST_TEST_MODULE Sorters Regress Test
#include <boost/test/unit_test.hpp>

#include "dictionary.h"
#include "dictionary_definer.h"

BOOST_AUTO_TEST_SUITE(sorters_alltogether)

	using dictionary_creator::criteria_dependent_sorters;

	struct TwoEntries
	{
		TwoEntries() : a{ new dictionary_creator::Entry("first") }, b{ new dictionary_creator::Entry("second") }
		{
			a->increment_counter();
		}

		bool define_entries() const
		{
			auto definer = [] (dictionary_creator::utf8_string word)
			{
				return define_word(std::move(word), dictionary_creator::Language::English);
			};
			a->define(definer);
			b->define(definer);
			return a->is_defined() && b->is_defined();
		}

		std::shared_ptr<dictionary_creator::Entry> a;
		std::shared_ptr<dictionary_creator::Entry> b;
	};

	BOOST_FIXTURE_TEST_CASE(sorting_by_frequency, TwoEntries)
	{
		BOOST_TEST_CONTEXT("MostFrequent")
		{
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostFrequent)](a, b) == true);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostFrequent)](b, a) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostFrequent)](b, b) == false);
		}
		BOOST_TEST_CONTEXT("LeastFrequent")
		{
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastFrequent)](a, a) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastFrequent)](a, b) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastFrequent)](b, a) == true);
		}
	}

	BOOST_FIXTURE_TEST_CASE(sorting_by_length, TwoEntries)
	{
		BOOST_TEST_CONTEXT("Longest")
		{
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Longest)](a, b) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Longest)](b, a) == true);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Longest)](a, a) == false);
		}
		BOOST_TEST_CONTEXT("Shortest")
		{
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Shortest)](a, b) == true);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Shortest)](b, a) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Shortest)](a, a) == false);
		}
		BOOST_TEST_CONTEXT("UTF8_aware_length")
		{
			auto ca = std::make_shared<dictionary_creator::Entry>(u8"ça");
			auto va = std::make_shared<dictionary_creator::Entry>(u8"va");
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Longest)](ca, va) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::Shortest)](ca, va) == false);
		}
	}

	BOOST_FIXTURE_TEST_CASE(sorting_by_ambiguity, TwoEntries, * boost::unit_test::disabled())
	{
		BOOST_TEST_REQUIRE(define_entries());

		BOOST_TEST_CONTEXT("MostAmbiguous")
		{
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostAmbiguous)](a, a) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostAmbiguous)](a, b) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::MostAmbiguous)](b, a) == true);
		}
		BOOST_TEST_CONTEXT("LeastAmbiguous")
		{
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastAmbiguous)](a, a) == false);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastAmbiguous)](a, b) == true);
			BOOST_TEST_CHECK(criteria_dependent_sorters[static_cast<size_t>(dictionary_creator::ComparisonType::LeastAmbiguous)](b, a) == false);
		}
	}

	BOOST_FIXTURE_TEST_CASE(default_sorting, TwoEntries)
	{
		dictionary_creator::DefaultEntrySorter sorter;

		BOOST_TEST_CONTEXT("basic sorting")
		{
			BOOST_TEST_CHECK(sorter(a, b) == true);
			BOOST_TEST_CHECK(sorter(b, a) == false);
			BOOST_TEST_CHECK(sorter(a, a) == false);
		}

		BOOST_TEST_CONTEXT("transparent sorting")
		{
			BOOST_TEST_CHECK(sorter(a, "zoo") == true);
			BOOST_TEST_CHECK(sorter(a, "america") == false);
			BOOST_TEST_CHECK(sorter(a, "first") == false);

			BOOST_TEST_CHECK(sorter("season", b) == true);
			BOOST_TEST_CHECK(sorter("seizure", b) == false);
			BOOST_TEST_CHECK(sorter("second", b) == false);
		}
	}

BOOST_AUTO_TEST_SUITE_END()
