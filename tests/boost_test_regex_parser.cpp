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
		BOOST_TEST_CHECK(rom_sub.size() == 20u);
	}

	BOOST_AUTO_TEST_CASE(english_uppercase_words)
	{
		pcre_parser::RegexParser capitalized(R"(\b[A-Z][a-z]*\b)");
		pcre_parser::RegexParser uppercase(R"(\b[A-Z]+\b)");

		BOOST_TEST_CHECK(capitalized.all_matches("this string contains exactly none cause ACRONYMS COUNTS NOT").empty());
		BOOST_TEST_CHECK(uppercase.all_matches("this string contains exactly none").empty());

		const auto capitalized_1_uppercase_2 = "193WORD <> SOMETHING848 @@@ Xnopyt-4859 thiS doe5 n0+ c0uN+ BUT UPPERCASE d03s";
		auto two_uppercase = uppercase.all_matches(capitalized_1_uppercase_2);
		BOOST_TEST_CHECK(two_uppercase.size() == 2u);
		BOOST_TEST_CHECK(two_uppercase.count("BUT") == 1u);
		BOOST_TEST_CHECK(two_uppercase.count("UPPERCASE") == 1u);
		BOOST_TEST_CHECK(capitalized.all_matches(capitalized_1_uppercase_2).size() == 1u);

		auto cpp = capitalized.all_matches(s_ex::cpp_program);
		BOOST_TEST_CHECK(cpp.count("Following") == 1u);
		BOOST_TEST_CHECK(cpp.size() == 1u);

		auto eng_sub_1 = uppercase.all_matches(s_ex::eng_subtitles_1);
		BOOST_TEST_CHECK(eng_sub_1.size() == 1u);

		auto eng_sub_2 = uppercase.all_matches(s_ex::eng_subtitles_2);
		BOOST_TEST_CHECK(eng_sub_2.size() == 3u);
		BOOST_TEST_CHECK(eng_sub_2.count("POOP") == 2u);
		BOOST_TEST_CHECK(eng_sub_2.count("I") == 1u);

		auto eng_sub_3 = capitalized.all_matches(s_ex::eng_subtitles_3);
		BOOST_TEST_CHECK(eng_sub_3.size() == 8u);

		auto rom_sub = capitalized.all_matches(s_ex::rom_subtitles);
		BOOST_TEST_CHECK(rom_sub.size() == 4u);
	}

	BOOST_AUTO_TEST_CASE(romanian_words)
	{
		pcre_parser::RegexParser words(u8R"(\b[A-ZĂÂÎȘŞȚŢ]?[a-zăâîșşțţ]+\b|\b[A-ZĂÂÎȘŞȚŢ]\b)");

		auto empty = words.all_matches("193 <> 848 @@@ 4859 th1s d0es n0+ c0uN+ NOR UPPERCASE d03s");
		BOOST_TEST_CHECK(empty.empty());

		BOOST_TEST_INFO("Romanian text example");
		auto subtitles = words.all_matches(s_ex::rom_subtitles);
		BOOST_TEST_CHECK(subtitles.size() == 29u);

		BOOST_TEST_INFO("Valid for all english texts as well");
		auto eng = words.all_matches(s_ex::eng_subtitles_3);
		BOOST_TEST_CHECK(eng.size() == 39u);
	}

	BOOST_AUTO_TEST_CASE(french_words)
	{
		pcre_parser::RegexParser words(u8R"(\b[A-ZÀÂÆÇÈÉÊËÏÎÔŒÙÛÜŸ]?[a-zàâæçèéêëïîôœùûüÿ]+\b)");

		BOOST_TEST_CHECK(words.all_matches("IL N_Y_A pAS d3 m0ts").empty());

		auto subtitles = words.all_matches(s_ex::fr_subtitles);
		BOOST_TEST_CHECK(subtitles.size() == 46u);
		BOOST_TEST_CHECK(subtitles.count(u8"répugne") == 1u);
		BOOST_TEST_CHECK(subtitles.count(u8"à") == 2u);
		BOOST_TEST_CHECK(subtitles.count(u8"Bête") == 1u);
		BOOST_TEST_CHECK(subtitles.count(u8"très") == 1u);

		auto pangram = words.all_matches(s_ex::french_pangram);
		BOOST_TEST_CHECK(pangram.size() == 66u);
		BOOST_TEST_CHECK(pangram.count(u8"ovoïde") == 1u);
		BOOST_TEST_CHECK(pangram.count(u8"ambiguë") == 1u);
		BOOST_TEST_CHECK(pangram.count(u8"côté") == 1u);
		BOOST_TEST_CHECK(pangram.count(u8"cænogénèse") == 1u);
		BOOST_TEST_CHECK(pangram.count(u8"à") == 3u);

		auto false_positive = words.all_matches(u8"wißen жалюзи freeşțylo");
		BOOST_TEST_CHECK(false_positive.empty());
	}

	BOOST_AUTO_TEST_CASE(russian_words)
	{
		pcre_parser::RegexParser every(u8R"(\b[А-ЯЁ]?[а-яё]+\b)");
		pcre_parser::RegexParser lowercase(u8R"(\b[а-яё]+\b)");

		auto no_lowercase = every.all_matches(u8R"(Нет НИ ОДНОГО сЛОВа В НиЖнЕм Регистре!)");
		BOOST_TEST_CHECK(no_lowercase.size() == 2u);

		BOOST_TEST_INFO("lowercase pattern makes no false positives");
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
		BOOST_TEST_CHECK(rus_sub_2_all.size() == 24u);
	}

	BOOST_AUTO_TEST_CASE(german_words)
	{
		pcre_parser::RegexParser words(u8R"(\b[A-ZẞÄÜÖa-zßäüö]?[a-zßäüö]+\b)");

		auto pangram = words.all_matches(u8"„Fix, Schwyz!“, quäkt Jürgen blöd vom Paß.");
		BOOST_TEST_CHECK(pangram.size() == 7u);

		BOOST_TEST_CHECK(words.all_matches(u8"Keine falsche positives без ложных срабатываний 3trois tronçonnement").size() == 3u);

		auto subtitles = words.all_matches(s_ex::de_subtitles);
		BOOST_TEST_CHECK(subtitles.size() == 26u);
		BOOST_TEST_CHECK(subtitles.count(u8"Über") == 1u);
		BOOST_TEST_CHECK(subtitles.count(u8"verfälscht") == 1u);

		auto text = words.all_matches(s_ex::nietzsche);
		BOOST_TEST_CHECK(text.size() == 91u);
		BOOST_TEST_CHECK(text.count(u8"der") == 6u);
		BOOST_TEST_CHECK(text.count(u8"ist") == 4u);
		BOOST_TEST_CHECK(text.count(u8"in") == 1u);
		BOOST_TEST_CHECK(text.count(u8"ein") == 2u);
	}

BOOST_AUTO_TEST_SUITE_END()
