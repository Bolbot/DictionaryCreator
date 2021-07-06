#define BOOST_TEST_MODULE Regex Parser Regress Test
#include <boost/test/unit_test.hpp>

#include <memory>
#include "string_examples.h"
#include "regex_parser.h"


BOOST_AUTO_TEST_SUITE(regex_parser)

	BOOST_AUTO_TEST_CASE(basics)
	{
		pcre_parser::RegexParser object("");
		BOOST_TEST_INFO("object size");
		BOOST_TEST_CHECK(sizeof(object) >= sizeof(std::unique_ptr<int>));	// since PIMPL we can't know more

		BOOST_TEST_INFO("throw upon malformed regex argument construction");
		BOOST_CHECK_THROW(pcre_parser::RegexParser("[invalid"), std::runtime_error);
	}

	BOOST_AUTO_TEST_CASE(searching_numbers)
	{
		pcre_parser::RegexParser numbers(u8R"(\b[0-9]+\b)");
		
		auto empty = numbers.all_matches(u8"there are no numbers, empty multiset shall be returned");
		
		BOOST_TEST_CHECK(empty.empty());
		
		auto n123 = numbers.all_matches(u8"Only 123 numbers n0t numb3rs w1th d1g1ts 234no, neither654.\n123, or 99, or 100!");
		
		BOOST_TEST_CHECK(n123.size() == 4u);
		BOOST_TEST_CHECK(n123.count(u8"123") == 2u);
		BOOST_TEST_CHECK(n123.count(u8"99") == 1u);
		BOOST_TEST_CHECK(n123.count(u8"100") == 1u);
	}

	BOOST_AUTO_TEST_CASE(english_lowercase_words)
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

	BOOST_AUTO_TEST_CASE(romanian_words)
	{
		BOOST_TEST_CHECK(true);		// TODO: find out why this works while UTF-8 is still not supported
	/*
		pcre_parser::RegexParser words(u8R"((*UTF8)\b[A-ZĂÂÎȘŞȚŢ]?[a-zăâîșşțţ]+\b)");

		auto empty = words.all_matches("193 <> 848 @@@ 4859 th1s d0es n0+ c0uN+ NOR UPPERCASE d03s");

		BOOST_TEST_CHECK(empty.empty());

		auto subtitles = words.all_matches(s_ex::rom_subtitles);
		BOOST_TEST_CHECK(subtitles.size() == 29u);
	*/
	}

	BOOST_AUTO_TEST_CASE(russian_words)
	{
		BOOST_TEST_CHECK(true);
	
		// Issue: parser won't function correctly with UTF-8 characters

		const std::string russian_letter_needle = u8R"(\b[ку]+\b)";
		const std::string russian_letter_haystack = u8"куку говорит кукушка, куку";

		// redundant check if given characters are UTF-8, requires #include <iostream>
		/*
		using iterator_type = uint8_t;
		std::cout << "Needle:   '" << russian_letter_needle << "'\t(";
		for (iterator_type i: russian_letter_needle)
		{
			std::cout << static_cast<size_t>(i) << ' ';
		}
		std::cout << ")\n";
		std::cout << "Haystack: '" << russian_letter_haystack << "'\t\t\t(";
		for (iterator_type i: russian_letter_haystack)
		{
			std::cout << static_cast<size_t>(i) << ' ';
		}
		std::cout << ")\n";
		*/
	
		// Expected behaviour: needle is found twice in haystack
		// Observed behaviour: needle is not found at all
		pcre_parser::RegexParser parser(russian_letter_needle.data());
		auto match_that_letter = parser.all_matches(russian_letter_haystack);
	
		BOOST_TEST_CHECK(match_that_letter.size() == 2u);
		// TODO: find out why and fix this issue; then complete this test case
		// probably uncommenting following section should be a good starting point
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

BOOST_AUTO_TEST_SUITE_END()
