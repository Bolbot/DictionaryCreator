#define BOOST_TEST_MODULE Dictionary Regress Test
#include <boost/test/unit_test.hpp>

#include "dictionary.h"

BOOST_AUTO_TEST_SUITE(dictionary_alltogether)

	const std::initializer_list<dictionary_creator::utf8_string> english_words
	{
		"arcbishop", "bomb", "cat", "dark", "enormous", "fly", "govern", "holy", "inactive", "jolly",
		"knight", "love", "misery", "nap", "over", "pun", "question", "rubish", "satellite", "thumb",
		"ubiquitous", "victory", "worm", "xeroradiography", "yield", "zeal"
	};

	const std::initializer_list<dictionary_creator::utf8_string> russian_words
	{
		u8"атлет", u8"броня", u8"волк", u8"глава", u8"день", u8"если", u8"ёжик", u8"жесть", u8"зонтик",
		u8"истина", u8"йогурт", u8"каватина", u8"лучше", u8"метрополитен", u8"низина", u8"оружие",
		u8"профессионализм", u8"ручей", u8"силос", u8"танцевальный", u8"улучшение", u8"фанатично",
		u8"художник", u8"цыган", u8"честь", u8"шкаф", u8"щёлочь", u8"эротика", u8"юстиция", u8"ясно"
	};

	constexpr auto Dictionary_minimal_possible_size =
		sizeof(size_t) + 2 * sizeof(std::map<dictionary_creator::letter_type, std::set<std::shared_ptr<dictionary_creator::Entry>>>);

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

	auto fake_definer = [] (auto whatever)
	{
		dictionary_creator::definitions_t definitions;

		definitions["A"] = { { "B" }, { "C" }, { "D" } };
		definitions["One"] = { { "Two" }, { "Three" }, { "Four" } };

		return definitions;	
	};

	template <typename Container>
	bool contains_word(const Container &container, const dictionary_creator::utf8_string &word)
	{
		return std::find_if(container.begin(), container.end(),	
			[&word] (const auto &ptr) { return ptr->get_word() == word; }) != container.end();
	};

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

	BOOST_AUTO_TEST_CASE(basic_features)
	{
		dictionary_creator::Dictionary object(dictionary_creator::Language::English);

		BOOST_TEST_INFO("object size");
		BOOST_TEST_CHECK(sizeof(object) >= Dictionary_minimal_possible_size);

		BOOST_TEST_INFO("get_language()");
		BOOST_TEST_CHECK((object.get_language() == dictionary_creator::Language::English));

		BOOST_TEST_INFO("empty upon creation");
		BOOST_TEST_CHECK(object.total_words() == 0u);

		BOOST_TEST_INFO("lookup() in empty yields nullptr");
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

	BOOST_FIXTURE_TEST_CASE(subset_features, DictionaryObjects)
	{
		BOOST_TEST_CONTEXT("get_letter_entries()")
		{
			BOOST_TEST_INFO("Each letter is represented by a single entry");
			for (auto letter: dictionary_creator::utf8_string{ "ABCDEFGhijklmnopQRSTUVWxyz" })
			{
				BOOST_TEST_INFO("get_letter_entries(" << letter << ")");
				BOOST_TEST_CHECK(eng.get_letter_entries(dictionary_creator::letter_type(1, letter)).size() == 1u);
			}
		}

		BOOST_TEST_CONTEXT("get_undefined()")
		{
			BOOST_TEST_REQUIRE(eng.get_undefined().size() == 26u);
			eng.lookup("love")->define(fake_definer);
			BOOST_TEST_CHECK(eng.get_undefined().size() == 25u);
		}

		BOOST_TEST_CONTEXT("get_top() by length")
		{
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::Shortest, 3).size() == 3u);
			auto eng_shortest_4 = eng.get_top(dictionary_creator::ComparisonType::Shortest, 4);

			BOOST_TEST_CHECK(contains_word(eng_shortest_4, "cat"));
			BOOST_TEST_CHECK(contains_word(eng_shortest_4, "nap"));
			BOOST_TEST_CHECK(contains_word(eng_shortest_4, "fly"));
			BOOST_TEST_CHECK(contains_word(eng_shortest_4, "pun"));

			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::Longest, 1), "xeroradiography"));

			BOOST_TEST_INFO("Top 0 returns empty structure");
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::Shortest, 0).size() == 0u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::Longest, 0).size() == 0u);


			BOOST_TEST_INFO("If requested number exceeds the size, all entries are returned sorted");
			auto english_shortest = eng.get_top(dictionary_creator::ComparisonType::Shortest, 1'000'000);
			BOOST_TEST_CHECK(english_shortest.size() == 26u);
			auto english_longest = eng.get_top(dictionary_creator::ComparisonType::Longest, 1'000'000);
			BOOST_TEST_CHECK(english_longest.size() == 26u);
			// TODO: check they're actually sorted
		}

		BOOST_TEST_CONTEXT("get_top() by ambiguity")
		{
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::MostAmbiguous, 999).size() == 26u);
			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::MostAmbiguous, 1), "love"));
			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::LeastAmbiguous, 25), "love") == false);

			BOOST_TEST_INFO("Top 0 returns empty structure");
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::MostAmbiguous, 0).size() == 0u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::LeastAmbiguous, 0).size() == 0u);

			BOOST_TEST_INFO("If requested number exceeds the size, all entries are returned sorted");
			// TODO: check they're actually sorted
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::MostAmbiguous, 1'000'000).size() == 26u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::LeastAmbiguous, 1'000'000).size() == 26u);
		}

		BOOST_TEST_CONTEXT("get_top() by frequency")
		{
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::MostFrequent, 1).size() == 1u);
			eng.lookup("ubiquitous")->increment_counter();
			eng.lookup("love")->increment_counter();
			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::MostFrequent, 7), "ubiquitous") == true);
			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::MostFrequent, 7), "love") == true);
			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::LeastFrequent, 7), "ubiquitous") == false);
			BOOST_TEST_CHECK(contains_word(eng.get_top(dictionary_creator::ComparisonType::LeastFrequent, 7), "love") == false);

			BOOST_TEST_INFO("Top 0 returns empty structure");
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::MostFrequent, 0).size() == 0u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::LeastFrequent, 0).size() == 0u);

			BOOST_TEST_INFO("If requested number exceeds the size, all entries are returned sorted");
			// TODO: check they're actually sorted
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::LeastFrequent, 1'000'000).size() == 26u);
			BOOST_TEST_CHECK(eng.get_top(dictionary_creator::ComparisonType::MostFrequent, 1'000'000).size() == 26u);
		}

		BOOST_TEST_CONTEXT("get_random_word() and get_random_words()")
		{
			BOOST_TEST_INFO("returns valid entry");
			BOOST_TEST_CHECK(eng.get_random_word().get() != nullptr);

			BOOST_TEST_INFO("number exact as requested or everything if request exceeds capacity");
			BOOST_TEST_CHECK(eng.get_random_words(11).size() == 11u);
			BOOST_TEST_CHECK(eng.get_random_words(0).size() == 0u);
			BOOST_TEST_CHECK(eng.get_random_words(1'000'000).size() == 26u);
		}
	}

	class DifficultyEntry : public dictionary_creator::Entry
	{
	public:
		DifficultyEntry(dictionary_creator::utf8_string word, double difficulty = 1.0)
			: dictionary_creator::Entry{ std::move(word) }, difficulty{ difficulty }
		{}

		double get_difficulty() const
		{
			return difficulty;
		}
	private:
		double difficulty;
	};

	auto get_comp_less_difficult()
	{
		return std::function<bool(const DifficultyEntry &, const DifficultyEntry &)>
		{
			[](const DifficultyEntry &a, const DifficultyEntry &b)
			{
				return a.get_difficulty() < b.get_difficulty();
			} 
		};
	}

	auto get_comp_more_difficult()
	{
		return std::function<bool(const DifficultyEntry &, const DifficultyEntry &)>
		{
			[](const DifficultyEntry &a, const DifficultyEntry &b)
			{
				return a.get_difficulty() > b.get_difficulty();
			} 
		};
	}

	BOOST_FIXTURE_TEST_CASE(custom_types, DictionaryObjects)
	{
		BOOST_TEST_CONTEXT("add_word<CustomType>()")
		{
			BOOST_TEST_CHECK(eng.add_word<DifficultyEntry>("red", 0.1));
			eng.add_word<DifficultyEntry>("green", 0.15);
			eng.add_word<DifficultyEntry>("blue", 0.12);
			eng.add_word<DifficultyEntry>("antidisestablishmentarianism", 3.5);
			BOOST_TEST_CHECK(eng.total_words() == 30u);

			BOOST_TEST_INFO("previously present word is just upgraded");
			eng.add_word<DifficultyEntry>("satellite", 1.2);
			BOOST_TEST_CHECK(eng.total_words() == 30u);
		}

		BOOST_TEST_CONTEXT("custom comparator get_top() least difficult")
		{
			BOOST_TEST_CHECK(contains_word(eng.get_top(get_comp_less_difficult(), 3), "red"));
			BOOST_TEST_CHECK(contains_word(eng.get_top(get_comp_less_difficult(), 3), "green"));
			BOOST_TEST_CHECK(contains_word(eng.get_top(get_comp_less_difficult(), 3), "blue"));
			BOOST_TEST_CHECK(contains_word(eng.get_top(get_comp_less_difficult(), 3), "antidisestablishmentarianism") == false);
			BOOST_TEST_CHECK(eng.get_top(get_comp_less_difficult(), 3).front()->get_word() == "red");
		}

		BOOST_TEST_CONTEXT("custom comparator get_top() most difficult")
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
			BOOST_TEST_INFO("At least as many as have been put there");
			BOOST_TEST_CHECK(eng.get_top(get_comp_less_difficult(), 5).back().get() != nullptr);

			BOOST_TEST_INFO("Not more than have been put there");
			BOOST_TEST_CHECK(dynamic_cast<DifficultyEntry *>(eng.get_top(get_comp_less_difficult(), 6).front().get()) != nullptr);
			BOOST_TEST_CHECK(dynamic_cast<DifficultyEntry *>(eng.get_top(get_comp_less_difficult(), 6).back().get()) == nullptr);

			BOOST_TEST_INFO("State updates upon another add");
			eng.add_word<DifficultyEntry>("pun", 0.1);
			BOOST_TEST_CHECK(dynamic_cast<DifficultyEntry *>(eng.get_top(get_comp_less_difficult(), 6).back().get()) != nullptr);
		}
	}

	BOOST_FIXTURE_TEST_CASE(big_five, DictionaryObjects)
	{
		// rvalue source
		auto get_english = [this] () -> dictionary_creator::Dictionary { auto res = eng; return std::move(res); };

		eng.add_proper_noun("Anna");
		BOOST_TEST_CHECK(eng.total_words() == 26u);

		BOOST_TEST_CONTEXT("copy construction")
		{
			dictionary_creator::Dictionary object = eng;
			BOOST_TEST_CHECK(exports_words(object, english_words));

			object.add_word("Anna");
			BOOST_TEST_CHECK(object.total_words() == 26u + 1u);

			BOOST_TEST_INFO("Proper nouns from origin are effectively taken into account");
			object.remove_proper_nouns();
			BOOST_TEST_CHECK(object.total_words() == 26u);
			BOOST_TEST_CHECK(exports_word(object, "Anna") == false);
		}

		BOOST_TEST_CONTEXT("move construction")
		{
			dictionary_creator::Dictionary object = get_english();
			BOOST_TEST_CHECK(exports_words(object, english_words));

			object.add_word("Anna");
			BOOST_TEST_CHECK(exports_word(object, "Anna") == true);

			BOOST_TEST_INFO("Proper nouns from origin are effectively taken into account");
			object.remove_proper_nouns();
			BOOST_TEST_CHECK(object.total_words() == 26u);
			BOOST_TEST_CHECK(exports_word(object, "Anna") == false);
		}

		rus.add_word(u8"Василий");
		rus.add_proper_noun(u8"Василий");
		dictionary_creator::Dictionary object = eng;

		BOOST_TEST_CONTEXT("copy assignment")
		{
			BOOST_TEST_INFO("Language gets changed upon assignment");
			object = rus;
			BOOST_TEST_CHECK((object.get_language() == dictionary_creator::Language::Russian));
			BOOST_TEST_CHECK(exports_words(object, russian_words));
			BOOST_TEST_CHECK(exports_word(object, u8"Василий") == true);

			object.remove_proper_nouns();
			BOOST_TEST_CHECK(exports_word(object, u8"Василий") == false);
		}

		object.add_proper_noun(u8"Япония");

		BOOST_TEST_CONTEXT("move assignment")
		{
			dictionary_creator::Dictionary dest(dictionary_creator::Language::French);

			BOOST_TEST_INFO("Language gets changed upon assignment");
			dest = std::move(object);
			BOOST_TEST_CHECK((dest.get_language() == dictionary_creator::Language::Russian));

			BOOST_TEST_CHECK(exports_word(dest, u8"Василий") == false);
			BOOST_TEST_CHECK(exports_word(dest, u8"Япония") == false);

			dest.add_word(u8"Япония");
			BOOST_TEST_CHECK(exports_word(dest, u8"Япония") == true);
			BOOST_TEST_CHECK(exports_words(dest, russian_words));

			BOOST_TEST_CHECK("Proper noun from the origin of move assignment is effectively present");
			dest.remove_proper_nouns();
			BOOST_TEST_CHECK(exports_word(dest, u8"Япония") == false);
			BOOST_TEST_CHECK(exports_words(dest, russian_words));
		}

		BOOST_TEST_CONTEXT("self assignment")
		{
			BOOST_TEST_CHECK(exports_words(rus, russian_words) == true);
			BOOST_TEST_CHECK(exports_word(rus, u8"Василий") == true);

			rus = rus;

			BOOST_TEST_INFO("self-assignment doesn't change any state");
			BOOST_TEST_CHECK(exports_words(rus, russian_words) == true);
			BOOST_TEST_CHECK(exports_word(rus, u8"Василий") == true);

			rus.remove_proper_nouns();

			BOOST_TEST_INFO("proper nouns are not affected by it either");
			BOOST_TEST_CHECK(exports_words(rus, russian_words) == true);
			BOOST_TEST_CHECK(exports_word(rus, u8"Василий") == false);

			rus.add_word(u8"Екатерина");
			rus.add_proper_noun(u8"Екатерина");

			BOOST_TEST_CHECK(exports_word(rus, u8"Екатерина") == true);

			// no robustness against self move-assignement should've been intended actually
			// see the rule of zero; therefore following failure is plausible and shouldn't be tested against
			//
			//auto size_before_self_assignment = rus.total_words();
			//rus = std::move(rus);
			//BOOST_TEST_CHECK(rus.total_words() == size_before_self_assignment);

			BOOST_TEST_INFO("no previous state remains");
			rus = eng;
			BOOST_TEST_CHECK(exports_words(rus, english_words) == true);
			BOOST_TEST_CHECK(exports_word(rus, u8"Василий") == false);
			BOOST_TEST_CHECK(exports_word(rus, u8"Екатерина") == false);

			rus.add_word(u8"Василий");
			rus.add_word(u8"Екатерина");
			rus.remove_proper_nouns();

			BOOST_TEST_INFO("proper nouns were gone upon assignment");
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

	BOOST_FIXTURE_TEST_CASE(operations, DictionaryObjectsLight)
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

			BOOST_TEST_INFO("proper nouns from both are affecting upon merge");
			BOOST_TEST_CHECK(contains(abcd_, "abgh"));
			BOOST_TEST_CHECK(contains_none(abcd_, "cdef"));

			BOOST_TEST_INFO("argument of merge is not affected");
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

			BOOST_TEST_INFO("proper nouns from both are affecting upon merge");
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

			BOOST_TEST_INFO("proper nouns from both are affecting upon merge");
			BOOST_TEST_CHECK(contains(abcde, "ab"));
			BOOST_TEST_CHECK(contains_none(abcde, "cdefgh"));

			BOOST_TEST_INFO("argument of merge is not affected");
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

			BOOST_TEST_INFO("proper nouns from both are affecting upon merge");
			BOOST_TEST_CHECK(contains(abcde, "ab"));
			BOOST_TEST_CHECK(contains_none(abcde, "cdefgh"));
		}

		BOOST_TEST_CONTEXT("compound subtract")
		{
			auto ijkl_ = abcdefghijkl;

			ijkl_.subtract(abcd).subtract([this] { auto res = efgh; return std::move(res); }());

			BOOST_TEST_INFO("both lvalue and rvalue proper nouns affect eventual state");
			BOOST_TEST_CHECK(contains(ijkl_, "ijkl"));
			BOOST_TEST_CHECK(contains_none(ijkl_, "abcdefgh"));

			BOOST_TEST_INFO("lvalue arguments are not affected");
			BOOST_TEST_CHECK(contains(abcd, "abcd"));
			BOOST_TEST_CHECK(contains_none(abcd, "efghijkl"));
		}

		BOOST_TEST_CONTEXT("self subtract")
		{
			auto selfsubtract = abcd;
			selfsubtract.add_proper_noun("e");
			selfsubtract.add_proper_noun("f");
			selfsubtract.add_proper_noun("g");

			BOOST_TEST_INFO("no entry remains after self subtract");
			selfsubtract.subtract(selfsubtract);
			BOOST_TEST_CHECK(selfsubtract.total_words() == 0u);

			BOOST_TEST_INFO("proper nouns are retained");
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

			BOOST_TEST_INFO("proper nouns from both sides affect the result");
			BOOST_TEST_CHECK(contains(abcdef.intersection_with(efghi), "e"));
			BOOST_TEST_CHECK(contains_none(abcdef.intersection_with(efghi), "abcdfghi"));

			BOOST_TEST_INFO("operation is commutative");
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

			BOOST_TEST_INFO("all proper nouns affect the eventual state");
			cdefgh.add_proper_noun("c");
			auto e = abcdef.intersection_with(cdefgh).intersection_with([&bcde]
					{
						auto res = bcde;
						res.add_proper_noun("d");
						return std::move(res);
					}());
			BOOST_TEST_CHECK(contains(e, "e"));
			BOOST_TEST_CHECK(contains_none(e, "abcdfghi"));

			BOOST_TEST_INFO("lvalue argument isn't getting affected");
			BOOST_TEST_CHECK(contains(cdefgh, "cdefgh"));
			BOOST_TEST_CHECK(contains_none(cdefgh, "abij"));
			cdefgh.remove_proper_nouns();
			BOOST_TEST_CHECK(contains(cdefgh, "defgh"));
			BOOST_TEST_CHECK(contains_none(cdefgh, "abcij"));
		}

		BOOST_TEST_CONTEXT("self intersection")
		{
			auto selfintersect = abcd;
			selfintersect.add_proper_noun("a");
			selfintersect.add_proper_noun("h");

			auto selfintersect_result = selfintersect.intersection_with(selfintersect);

			BOOST_TEST_INFO("argument stays unaffected");
			BOOST_TEST_CHECK(contains(selfintersect, "abcd"));

			BOOST_TEST_INFO("result is new object affeted by proper nouns");
			BOOST_TEST_CHECK(contains(selfintersect_result, "bcd"));
			BOOST_TEST_CHECK(selfintersect_result.total_words() == 3u);

			selfintersect_result.add_word("h");
			selfintersect_result.add_word("i");
			BOOST_TEST_CHECK(selfintersect_result.total_words() == 5u);

			BOOST_TEST_INFO("previously gathered proper nouns influence the eventual state");
			selfintersect_result.remove_proper_nouns();
			BOOST_TEST_CHECK(contains(selfintersect_result, "bcdi"));
			BOOST_TEST_CHECK(contains_none(selfintersect_result, "aefgh"));
		}
	}

	BOOST_FIXTURE_TEST_CASE(arithmetics, DictionaryObjectsLight)
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

BOOST_AUTO_TEST_SUITE_END()
