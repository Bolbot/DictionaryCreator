#define BOOST_TEST_MODULE Dictionary Entry Regress Test
#include <boost/test/unit_test.hpp>

#include "dictionary_entry.h"

BOOST_AUTO_TEST_SUITE(dictionary_entry)

	BOOST_AUTO_TEST_CASE(dictionary_entry_alltogether)
	{
		constexpr auto DictionaryEntry_minimal_possible_size =
				sizeof(dictionary_creator::utf8_string)
				+ sizeof(dictionary_creator::definitions_t)
				+ sizeof(size_t) + sizeof(bool);
		const dictionary_creator::utf8_string word{ "antidisestablismentarianism" };

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

			auto fake_definer = [] (auto whatever)
			{
				if constexpr (sizeof(whatever) == -1)
				{
					throw std::runtime_error("Thanks Microsoft for \"fixed\" C2760 false positive on [[ maybe_unused ]]");
				}

				dictionary_creator::definitions_t definitions;

				definitions["A"] = { { "B" }, { "C" }, { "D" } };
				definitions["One"] = { { "Two" }, { "Three" }, { "Four" } };

				return definitions;	
			};

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

BOOST_AUTO_TEST_SUITE_END()
