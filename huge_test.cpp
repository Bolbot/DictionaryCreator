#include "huge_test.h"

#include <bitset>

void huge_test::dm_creation()
{
	dictionary_creator::DictionaryManager eng(dictionary_creator::Language::English);
	dictionary_creator::DictionaryManager rus(dictionary_creator::Language::Russian);

	std::cout << "Creation:\n\t" << typeid(eng).name() << ", size " << sizeof(eng) << std::endl;
}

void huge_test::special_cases()
{
	std::cout << "Special case 1. The same word many times\n";

	std::string same_word_filename = "HUGE_TEST_source_same_word_many_times.txt";

	constexpr size_t number = 10'080'999;

	dictionary_creator::utf8_string word = u8"buffalo";

	if (std::ofstream output(same_word_filename); output.good())
	{
		size_t current = 0;

		while (current != number)
		{
			if (current % 13 == 0)
			{
				output << "\nSo, ";
			}
			else if (current % 17 == 0)
			{
				output << "not. And ";
			}
			else if (current % 29 == 0)
			{
				output << "here? Yes! Now ";
			}

			output << word << ' ';

			++current;
		}

		output << std::endl;
	}

	dictionary_creator::DictionaryManager eng(dictionary_creator::Language::English);

	eng.add_input_file(same_word_filename);

	auto parse_task = [&eng]
	{
		eng.parse_all_pending();
	};
	auto parse_time = huge_test::execution_time(parse_task);

	std::cout << "\tParsing text containing " << number << " \"" << word << "\" took "
		<< parse_time.count() << " milliseconds.\n"
		<< "\tResult is " << (eng.lookup_or_add_word(word)->get_counter() == number ? "exact" : "flawed")
		<< "\tcounter says it's " << eng.lookup_or_add_word(word)->get_counter() << " occurences of \'" << word << '\''
		<< std::endl;
}

dictionary_creator::letter_type get_next_letter(dictionary_creator::letter_type letter)
{
	auto first_octet = static_cast<size_t>(letter.front());
	size_t bytes = 0;
	for (size_t i = 1 << 7; (first_octet & i) != 0ull; i >>= 1)
	{
		++bytes;
	}
	if (bytes == 0)
	{
		bytes = 1;
	}

//	std::cout << "get_next_letter(" << letter << ")\t\tsize of it is " << letter.size() << "\tbytes: " << bytes << std::endl;

	size_t target = 0;
	size_t shift = 0;
	for (size_t i = bytes - 1; i != 0; --i)
	{
		auto octet = static_cast<size_t>(letter[i]);

		if ((octet & 0xC0) != 0x80)
		{
			throw std::runtime_error("broken UTF-8 letter");
		}
		octet &= 0x3F;
		octet <<= shift;
		target |= octet;

		shift += 6;
	}

	size_t bits_in_first_octet = 8 - (bytes + 1);
	size_t first_octet_mask = 0;
	for (size_t i = 0; i != bits_in_first_octet; ++i)
	{
		first_octet_mask |= (1 << i);
	}
	if (bytes == 1)
	{
		first_octet_mask = 0x7F;
	}
	first_octet &= first_octet_mask;

	target |= (first_octet << shift);

//	std::cout << "\n\t\t\tDEC: " << std::dec << target << "\tHEX: " << std::hex << target << "\tBIN: " << std::bitset<16>(target) << std::endl;

	++target;

	letter.clear();

	if (target <= 0x7F)
	{
		letter.push_back(target);
	}
	else if (target <= 0x7FF)
	{
		size_t first = (0xC0 | (target >> 6));
		size_t second = (0x80 | (target & 0x3F));
		letter.push_back(first);
		letter.push_back(second);
	}
	else
	{
		throw std::runtime_error("3+ byte UTF-8 letters are not yet supported");
	}

	return letter;
}

std::set<std::string> prepare_words(dictionary_creator::letter_type first = u8"a", dictionary_creator::letter_type last = u8"z")
{
	std::set<std::string> words;

	auto plus_one_letter = [first, last] (const std::string &s)
	{
		std::set<std::string> temp;
		for (auto i = first; i <= last; i = get_next_letter(i))
		{
			temp.insert(s + i);
		}
		return temp;
	};

	words.insert(std::string{});

	for (size_t i = 0; i != 4; ++i)
	{
		auto old_set = words;

		for (const auto &i: old_set)
		{
			words.merge(plus_one_letter(i));
		}
	}

	return words;
}

std::set<std::string> prepare_more_words(const std::set<std::string> &words, size_t number, char from = 'a', char to = 'z')
{
	std::set<std::string> set;

	auto add_a_bit_more = [&words, &number, &set] (char ch)
	{
		for (const auto &i: words)
		{
			if (set.size() >= number)
			{
				return;
			}
			set.insert(i + ch);
		}
	};

	for (char i = from; i <= to; ++i)
	{
		add_a_bit_more(i);
	}

	return set;
}

class EnglishTestSubtype : public dictionary_creator::Entry
{
public:
	EnglishTestSubtype(dictionary_creator::utf8_string str = "uninitialized", int s = int{}, double b = double{})
	       : dictionary_creator::Entry{ std::move(str) }, s{ s }, b{ static_cast<int>(b) }
	{}
	int s;
	int b;

	template <typename A>
	void serialize(A &ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<dictionary_creator::Entry>(*this);
		ar & s;
		ar & b;
	}
};

BOOST_CLASS_EXPORT_GUID(EnglishTestSubtype, typeid(EnglishTestSubtype).name())

void huge_test::main_huge_test_english()
{
	std::cout << "\nHUGE TEST for DictionaryManager lib and features. English version.\n" << std::endl;

	//dm_creation();
	auto basic_words = prepare_words();
	auto first_set = prepare_more_words(basic_words, 1'000'000, 'k', 'q');
	auto second_set = prepare_more_words(basic_words, 1'000'000, 's', 'z');

	auto print_to_file = [] (const std::set<std::string> &set, std::string file_name)
	{
		if (auto output = std::ofstream(file_name); output.good())
		{
			size_t nl = 0;
			for (const auto &i: set)
			{
				output << i << (++nl % 10 ? '\t' : '\n');
			}
			output << std::endl;
		}
		std::cout << "\tSet of " << set.size() << " words is written to " << file_name << std::endl;
	};

	std::string source_1 = "HUGE_TEST_source_file_one.txt";
	std::string source_2 = "HUGE_TEST_source_file_two.txt";
	//
	// 1. Parsing
	//
	std::cout << "\n1. Parsing\n\n";
	print_to_file(first_set, source_1);
	print_to_file(second_set, source_2);

	dictionary_creator::DictionaryManager english(dictionary_creator::Language::English);
	english.add_input_file(source_1);
	english.add_input_file(source_2);

	auto parse_all_task = [&english] () { english.parse_all_pending(); };
	auto parse_all_time = huge_test::execution_time(parse_all_task);
	std::cout << "\tParsing test done\n";
	//
	// 2. Contains check
	//
	std::cout << "\n2. Contains check\n\n";
	bool perfect = true;
	auto contains_check_task = [&perfect, &english, &first_set, &second_set]
	{
		auto check = [&perfect, &english] (const std::set<std::string> &set)
		{
			for (const auto &i: set)
			{
				bool contains = english.contains_word(i);
				bool goodsize = i.size() >= 3;

				if (contains ^ goodsize)
				{
					std::cout << "\t\tError: " << i << " [" << i.size() << "] is"
						<< (contains ? "" : "n't") << " in the dictionary\n";
					perfect = false;
				}
			}
		};
		check(first_set);
		check(second_set);
	};
	auto contains_check_time = huge_test::execution_time(contains_check_task);
	if (perfect)
	{
		std::cout << "\tWords from both set (" << (first_set.size() + second_set.size())
			<< "in total) are contained in dictionary" << std::endl;
	}
	else
	{
		std::cout << "\tError: some words were missing" << std::endl;
	}
	std::cout << "\tContains check done\n";
	//
	// 3. Letter subset
	//
	std::cout << "\n3. Letter subset\n\n";
	perfect = true;
	auto starting_match_task = [&english, &perfect] ()
	{
		std::string first_letter = " ";
		for (char i = 'a'; i <= 'z'; ++i)
		{
			first_letter.front() = i;
			auto letter_subset = english.get_subset(first_letter);

			if (letter_subset.empty())
			{
				std::cout << "\t\tEmpty subset starting from letter " << i << '\n';
				perfect = false;
			}

			for (const auto &word: letter_subset)
			{
				if (word->get_word().front() != i)
				{
					std::cout << "\t\tError: in subset for letter " << first_letter
						<< " there is a word " << word->get_word()
					       	<< ", starting with " << word->get_word().front() << '\n';
					perfect = false;
				}
			}
		}
	};
	auto starting_match_time = huge_test::execution_time(starting_match_task);
	if (perfect)
	{
		std::cout << "\tReturned letter starting subset perfectly fine\n";
	}
	else
	{
		std::cout << "\tSome letter subset were flawed\n";
	}
	std::cout << "\tLetter subset test done\n";
	//
	// 4. Shortest words subset
	//
	std::cout << "\n4. Shortest words subset\n";
	perfect = true;
	auto under_5 = [&first_set, &second_set] ()
	{
		std::set<std::string> set;
		auto add = [&set] (const std::set<std::string> &s)
		{
			for (auto i: s)
			{
				if (2 < i.size() && i.size() < 5)
				{
					set.emplace(std::move(i));
				}
			}
		};
		add(first_set);
		add(second_set);
		return set;
	}().size();
	std::cout << "\t" << under_5 << " words should be < 5 letters long\n";

	dictionary_creator::subset_t shortest_words_subset;
	auto shortest_subset_test = [&english, &shortest_words_subset, under_5] ()
	{
		shortest_words_subset = english.get_subset(dictionary_creator::ComparisonType::Shortest, under_5);
	};
	auto shortest_subset_time = huge_test::execution_time(shortest_subset_test);

	if (shortest_words_subset.size() != under_5)
	{
		perfect = false;
		std::cout << "\tError: shortest words subset size = " << shortest_words_subset.size() << u8" \U00002260 "
			<< under_5 << " = requested for that subset words" << std::endl;
	}
	for (const auto &i: shortest_words_subset)
	{
		if (i->get_word().size() >= 5)
		{
			std::cout << "\t\tError: [" << i->get_word().size() << "] letter word returned as one of "
				<< under_5 << " shortest words\n";
			perfect = false;
		}
	}
	std::cout << "\tShortest words subset test done " << (perfect ? "perfectly" : "with flaws") << std::endl;
	//
	// 5. Longest words subset
	//
	std::cout << "\n5. Longest words subset\n\n";
	auto third_set = [&first_set, &second_set] (size_t number)
	{
		srand(time(nullptr));

		std::set<std::string> set;

		size_t counter = 0;
		while (set.size() < number)
		{
			auto first_it = first_set.begin();
			std::advance(first_it,
					dictionary_creator::random_number(first_set.size())
	//			//	counter
					);
			auto second_it = second_set.begin();
			std::advance(second_it,
					dictionary_creator::random_number(second_set.size())
	//			//	counter
					);
			++counter;

			std::string newstring = { *first_it + *second_it };

			auto [iter, res] = set.insert(newstring);
			if (!res)
			{
				std::cout << "\t\tError: \"" << newstring << "\" not inserted" << std::endl;
			}
			else
			{
				std::cout << (counter % 100 ? "" : ".") << std::flush;
				if (counter % 10'000 == 0)
				{
					std::cout << " " << counter << " / " << number << std::endl;
				}
			}
		}
		return set;
	}(10'000);

	std::string source_3 = "HUGE_TEST_source_file_three.txt";
	print_to_file(third_set, source_3);
	english.add_input_file(source_3);
	english.parse_all_pending();

	perfect = true;
	auto over_5 = std::count_if(third_set.begin(), third_set.end(), [] (const std::string &s) { return dictionary_creator::utf8_length(s) > 5; });
	dictionary_creator::subset_t longest_words_subset;

	auto longest_subset_test = [&english, &longest_words_subset, over_5]
	{
		longest_words_subset = english.get_subset(dictionary_creator::ComparisonType::Longest, over_5);
	};
	auto longest_subset_time = huge_test::execution_time(longest_subset_test);

	if (longest_words_subset.size() != over_5)
	{
		perfect = false;
		std::cout << "\tError: longest words subset size = " << longest_words_subset.size() << u8" ≠ "
			<< over_5 << " = requested for longest subset number" << std::endl;
	}
	for (const auto &i: longest_words_subset)
	{
		if (i->get_word().size() <= 5)
		{
			std::cout << "\t\tError: [" << i->get_word().size() << "] letter word returned as one of "
				<< over_5 << " longest words\n";
			perfect = false;
		}
	}
	for (auto i: third_set)
	{
		if (!english.contains_word(i))
		{
			std::cout << "\tError: " << i << " somehow disappeared from the dictionary" << std::endl;
			perfect = false;
		}
	}
	std::cout << "\tLongest words subset done " << (perfect ? "perfectly" : "with flaws") << std::endl;
	//
	// 6. Custom subset
	//
	std::cout << "\n6. Custom subset\n\n";
	auto same_letter_first = [] (const dictionary_creator::Entry &a, const dictionary_creator::Entry &b)
	{
		auto same_letter = [] (const dictionary_creator::utf8_string &word)
		{
			for (auto i: word)
			{
				if (i != word.front())
				{
					return false;
				}
			}
			return true;
		};

		auto same_a = same_letter(a.get_word());
		auto same_b = same_letter(b.get_word());

		if (same_a && !same_b)
		{
			return true;
		}
		else if (same_b && !same_a)
		{
			return false;
		}
		else
		{
			return a.get_word() < b.get_word();
		}
	};
	auto same_letter = [] (const dictionary_creator::utf8_string &word)
	{
		for (auto i: word)
		{
			if (i != word.front())
			{
				return false;
			}
		}
		return true;
	};

	std::set<std::string> same_letter_words_from_sets;
	auto populate_same_letter_words_from_sets = [&same_letter, &same_letter_words_from_sets] (const std::set<std::string> &arg_set)
	{
		for (const auto &i: arg_set)
		{
			if (same_letter(i) && dictionary_creator::utf8_length(i) >= 3)
			{
				same_letter_words_from_sets.insert(i);
			}
		}
	};
	populate_same_letter_words_from_sets(first_set);
	populate_same_letter_words_from_sets(second_set);
	populate_same_letter_words_from_sets(third_set);
	auto different_same_letter_words = same_letter_words_from_sets.size();

	std::cout << "\t" << different_same_letter_words << " words have perfect match of each letter" << std::endl;

	dictionary_creator::subset_t same_letter_words_from_dictionary;
	auto custom_compare_task = [&english, &same_letter_first, &same_letter_words_from_dictionary, different_same_letter_words]
	{
		same_letter_words_from_dictionary = english.get_subset(same_letter_first, different_same_letter_words);
	};
	auto custom_compare_time = huge_test::execution_time(custom_compare_task);

	perfect = true;
	for (const auto &i: same_letter_words_from_dictionary)
	{
		if (!same_letter(i->get_word()))
		{
			std::cout << "\t\tError: custom comparator top contains word \"" << i->get_word()
				<< "\" that consits from different letters\n";
			perfect = false;
		}
	}
	std::cout << "\tCustom subset test done " << (perfect ? "perfectly" : "with flaws") << std::endl;
	//
	// 7. Add as subtype
	//
	std::cout << "\n7. Add as subtype\n\n";
	auto fourth_set = prepare_more_words(basic_words, 500'000, 'q', 't');
	auto add_subtype_task = [&fourth_set, &english] ()
	{
		for (auto i: fourth_set)
		{
			english.lookup_or_add_word<EnglishTestSubtype>(i,
					std::count(i.begin(), i.end(), 's'),
					100. * static_cast<double>(std::count(i.begin(), i.end(), 'b')) / static_cast<double>(i.size()));
		}
	};
	auto add_subtype_time = huge_test::execution_time(add_subtype_task);

	perfect = true;
	for (auto i: fourth_set)
	{
		if (!english.contains_word(i))
		{
			perfect = false;
			std::cout << "\tError: " << i << " isn't in dictionary" << std::endl;
		}
	}
	std::cout << "\tAdd words as Entry subtype done " << (perfect ? "perfectly" : "with flaws") << std::endl;
	//
	// 8-9. Subtype subsets
	//
	auto most_s_first = [] (const EnglishTestSubtype &a, const EnglishTestSubtype &b)
	{
		return (a.s > b.s) || (a.s == b.s && a.get_word() < b.get_word());
	};

	auto most_b_first = [] (const EnglishTestSubtype &a, const EnglishTestSubtype &b)
	{
		return (a.b > b.b) || (a.b == b.b && a.get_word() < b.get_word());
	};

	dictionary_creator::subset_t most_s_words;
	auto get_most_s_task = [&english, &most_s_words, &most_s_first] ()
	{
		most_s_words = english.get_subset(most_s_first, 500'000);
	};
	auto get_most_s_time = huge_test::execution_time(get_most_s_task);

	std::cout << "\n8. Subtype subset: most \'s\'\n\n";
	perfect = true;
	int check_s = dynamic_cast<EnglishTestSubtype *>(most_s_words.front().get())->s;
	for (auto i: most_s_words)
	{
		if (auto ab = dynamic_cast<EnglishTestSubtype *>(i.get()); ab)
		{
			if (ab->s > check_s)
			{
				std::cout << "\tError: " << ab->get_word() << " [" << ab->s << " > " << check_s << "]" << std::endl;
				perfect = false;
			}
			else if (ab->s < check_s)
			{
				check_s = ab->s;
			}
		}
		else
		{
			std::cout << "\tError: returned pointer won't dynamic cast, must be a wrong pointer" << std::endl;
			perfect = false;
		}
	}
	std::cout << "\tSubtype subset: most \'s\' done " << (perfect ? "perfectly" : "with flaws") << std::endl;

	dictionary_creator::subset_t most_b_words;
	auto get_most_b_task = [&english, &most_b_words, &most_b_first] ()
	{
		most_b_words = english.get_subset(most_b_first, 500'001);
	};
	auto get_most_b_time = huge_test::execution_time(get_most_b_task);

	std::cout << "\n9. Subtype subset: most \'b\'\n\n";
	perfect = true;
	double check_b = dynamic_cast<EnglishTestSubtype *>(most_b_words.front().get())->b;

	for (auto it = most_b_words.cbegin(); it != most_b_words.cend(); ++it)
	{
		if (auto ab = dynamic_cast<EnglishTestSubtype *>(it->get()); ab)
		{
			if (ab->b > check_b)
			{
				std::cout << "\tError: " << ab->get_word() << " [" << ab->b << " > " << check_b << "]" << std::endl;
				perfect = false;
			}
			else if (ab->b < check_b)
			{
				check_b = ab->b;
			}
		}
		else
		{
			std::cout << "\tError: pointer won't cast, improper custom subtype sorting result" << std::endl;
			perfect = false;
		}
	}
	std::cout << "\tSubtype subset: most \'b\' done " << (perfect ? "perfectly" : "with flaws") << std::endl;
	//
	// 10. Random words
	//
	std::cout << "\n10. Random words\n\n";
	dictionary_creator::subset_t random_words_vec;
	size_t random_words_number = 1'000'000;
	auto random_words_task = [&english, &random_words_number, &random_words_vec]
	{
		random_words_vec = english.get_random_words(random_words_number);
	};
	auto random_words_time = huge_test::execution_time(random_words_task);

	if (auto setsize = std::set<std::shared_ptr<dictionary_creator::Entry>>(random_words_vec.begin(), random_words_vec.end()).size();
		       setsize != random_words_vec.size())
	{
		std::cout << "\tError: some of the random words returned match\t" << setsize << " unique instead of "
			<< random_words_number << std::endl;
	}
	std::cout << "\tRandom words test done" << std::endl;
	//
	// 11. Default export
	//
	std::cout << "\n11. Default export\n\n";
	std::string output_1 = "HUGE_TEST_output_file_1.all_the_dictionary.txt";
	auto default_export_task = [&english, &output_1]
	{
		if (auto output = std::ofstream(output_1); output.good())
		{
			english.set_output(output);
			english.export_dictionary();
		}
	};
	auto default_export_time = huge_test::execution_time(default_export_task);

	if (std::ifstream(output_1).good())
	{
		std::cout << "\tExported all the dictionary in " << output_1 << std::endl;
	}
	std::cout << "\tDefault export done" << std::endl;
	//
	// 12. Export top
	//
	std::cout << "\n12. Export top\n\n";
	english.add_input_file(output_1);
	english.parse_all_pending();

	std::string output_2 = "HUGE_TEST_output_file_2.top_of_most_frequent_words.txt";
	auto export_frequent_task = [&english, &output_2]
	{
		if (auto output = std::ofstream(output_2); output.good())
		{
			english.set_output(output);
			english.export_top(dictionary_creator::ComparisonType::MostFrequent, 1'000'000);
		}
	};
	auto export_frequent_time = huge_test::execution_time(export_frequent_task);

	if (std::ifstream test(output_2); test.good())
	{
		std::cout << "\tExported top of most frequent words to " << output_2 << std::endl;
	}
	std::cout << "\tExport top most frequent words test is done" << std::endl;
	//
	// 13. Export subsets
	//
	std::cout << "\n13. Export subsets\n\n";
	std::string output_3 = "HUGE_TEST_output_file_3.subset_of_longest_words.txt";
	auto export_longest_task = [&english, &longest_words_subset, &output_3]
	{
		if (auto output = std::ofstream(output_3); output.good())
		{
			english.set_output(output);
			english.export_subset(longest_words_subset);
		}
	};
	auto export_longest_time = huge_test::execution_time(export_longest_task);

	if (std::ifstream test(output_3); test.good())
	{
		std::cout << "\tExported subset of longest words to " << output_3 << std::endl;
	}

	std::string output_4 = "HUGE_TEST_output_file_4.subset_shortest_w.txt";
	auto export_shortest_task = [&english, &subset = std::as_const(shortest_words_subset), &file = std::as_const(output_4)]
	{
		if (auto output = std::ofstream(file); output.good())
		{
			output << "\n" << std::endl;
			english.set_output(output);
			english.export_subset(subset);
		}
		else
		{
			std::cout << "\tError: cannot export to " << file << std::endl;
		}
	};
	auto export_shortest_time = huge_test::execution_time(export_longest_task);

	if (std::ifstream test(output_4); test.good())
	{
		std::cout << "\tExported subset of shortest words to " << output_4 << std::endl;
	}
	else
	{
		std::cout << "\tHaven't export shortest words subset. Reason is unknown" << std::endl;
	}

	std::string output_5 = "HUGE_TEST_output_file_5.words_of_same_letters.txt";
	auto export_sameletter_task = [&english, &subset = std::as_const(same_letter_words_from_dictionary), &file = std::as_const(output_5)]
	{
		if (auto output = std::ofstream(file); output.good())
		{
			english.set_output(output);
			english.export_subset(subset);
		}
	};
	auto export_sameletter_time = huge_test::execution_time(export_sameletter_task);

	if (std::ifstream test(output_5); test.good())
	{
		std::cout << "\tExported subset of same letter words to " << output_5 << std::endl;
	}

	std::string output_6 = "HUGE_TEST_output_file_6.most_s_first.txt";
	auto export_mostS_task = [&english, &subset = std::as_const(most_s_words), &file = std::as_const(output_6)]
	{
		if (auto output = std::ofstream(file); output.good())
		{
			english.set_output(output);
			english.export_subset(subset);
		}
	};
	auto export_mostS_time = huge_test::execution_time(export_mostS_task);

	if (std::ifstream test(output_6); test.good())
	{
		std::cout << "\tExported subset of most letter \"s\" containing words to " << output_6 << std::endl;
	}

	std::string output_7 = "HUGE_TEST_output_file_7.most_b_first.txt";
	auto export_mostB_task = [&english, &subset = std::as_const(most_b_words), &file = std::as_const(output_7)]
	{
		if (auto output = std::ofstream(file); output.good())
		{
			english.set_output(output);
			english.export_subset(subset);
		}
	};
	auto export_mostB_time = huge_test::execution_time(export_mostB_task);

	if (std::ifstream test(output_7); test.good())
	{
		std::cout << "\tExported subset of most letter \"b\" dominated words to " << output_7 << std::endl;
	}

	std::string output_8 = "HUGE_TEST_output_file_8.random_words.txt";
	auto export_random_task = [&english, &subset = std::as_const(random_words_vec), &file = std::as_const(output_8)]
	{
		if (auto output = std::ofstream(file); output.good())
		{
			english.set_output(output);
			english.export_subset(subset);
		}
	};
	auto export_random_time = huge_test::execution_time(export_random_task);

	if (std::ifstream test(output_8); test.good())
	{
		std::cout << "\tExported subset of random words to " << output_8 << std::endl;
	}
	//
	// 14. Serialization
	//
	std::cout << "\n14. Serialization\n\n";
	dictionary_creator::utf8_string serialization_filename = "HUGE_TEST_serialization file english";
	std::cout << "\tBefore rename: " << english.get_name() << '\n';
	english.rename(serialization_filename);
	std::cout << "\tAfter rename: " << english.get_name() << '\n';
	auto save_dictionary_task = [&english]
	{
		english.save_dictionary();
	};
	auto save_dictionary_time = huge_test::execution_time(save_dictionary_task);

	dictionary_creator::DictionaryManager english_read(dictionary_creator::Language::English);
	perfect = false;
	auto load_dictionary_task = [&english_read, &serialization_filename, &perfect]
	{
		for (const auto &name: dictionary_creator::available_dictionaries())
		{
			if (name.human_readable == serialization_filename)
			{
				english_read = dictionary_creator::load_dictionary(name.full);
				perfect = true;
				break;
			}
		}
		if (perfect == false)
		{
			std::cout << "\tError: failed to find the expected dictionary among the saved ones" << std::endl;
		}
	};
	auto load_dictionary_time = huge_test::execution_time(load_dictionary_task);
	
	perfect = true;
	{
		auto a_top = english.get_subset(dictionary_creator::ComparisonType::Shortest, 5'000);
		auto b_top = english_read.get_subset(dictionary_creator::ComparisonType::Shortest, 5'000);

		if (a_top.size() != b_top.size())
		{
			perfect = false;
			std::cout << "\tError: shortest subset mismatch between original and loaded dictionaries ("
				<< b_top.size() << " instead of expected " << a_top.size() << ")" << std::endl;
		}
		else
		{
			for (size_t i = 0; i != a_top.size(); ++i)
			{
				if (a_top[i]->get_word() != b_top[i]->get_word())
				{
					perfect = false;
					std::cout << "\tError: subset mismatch" << std::endl;
				}
			}
		}
	}
	std::cout << "\tSerialization done " << (perfect ? "perfectly" : "with flaws") << std::endl;
	// OUTPUT BENCHMARK RESULTS
	std::string resfile = "HUGE_TEST_benchmark_results.txt";
	std::ofstream benchmark_results(resfile);
	if (benchmark_results.good())
	{
		benchmark_results << "Benchmark results of HUGE TEST for DictionaryManager lib and features\n" << std::endl;

		benchmark_results << "1. Parsing.\n\t2 sets of files 1'000'000 possible words each.\n\t"
			<< parse_all_time.count() << " milliseconds.\n";

		benchmark_results << "2. Contains check.\n\tCheck if it contains all necessary and no unnecessary words.\n\t"
			<< contains_check_time.count() << " milliseconds.\n";

		benchmark_results << "3. Letter subset.\n\tReturned subset for each english letter and checked it.\n\t"
			<< starting_match_time.count() << " milliseconds.\n";

		benchmark_results << "4. Shortest subset.\n\tReturned and checked subset of " << under_5 << " shortest words.\n\t"
			<< shortest_subset_time.count() << " milliseconds.\n";

		benchmark_results << "5. Longest subset.\n\tReturned and checked subset of " << over_5 << " longest words.\n\t"
			<< longest_subset_time.count() << " milliseconds.\n";

		benchmark_results << "6. Custom subset.\n\tReturned subset of " << different_same_letter_words << " words via custom comparator.\n\t"
			<< custom_compare_time.count() << " milliseconds.\n";

		benchmark_results << "7. Add as subtype.\n\tAdded " << fourth_set.size() << " new words as Entry derrived type pointers.\n\t"
			<< add_subtype_time.count() << " milliseconds.\n";

		benchmark_results << "8. Subtype subset.\n\tRetrieved subtype custom sorted subset of " << most_s_words.size() << " most s words.\n\t"
			<< get_most_s_time.count() << " milliseconds.\n";

		benchmark_results << "9. Subtype subset.\n\tRetrieved subtype custom sorted subset of " << most_b_words.size() << " most b words.\n\t"
			<< get_most_b_time.count() << " milliseconds.\n";

		benchmark_results << "10. Random subset.\n\tRetrieved subset of " << random_words_number << " random words.\n\t"
			<< random_words_time.count() << " milliseconds.\n";

		benchmark_results << "11. Default export.\n\tExported all the dictionary with default export settings.\n\t"
			<< default_export_time.count() << " milliseconds.\n";

		benchmark_results << "12. Export top.\n\tExported top 1'000'000 most frequent words.\n\t"
			<< export_frequent_time.count() << " milliseconds.\n";

		benchmark_results << "13. Export subset.\n\t"
			<< longest_words_subset.size() << " longest words.\n\t\t" << export_longest_time.count() << " milliseconds.\n\t"
			<< shortest_words_subset.size() << " shortest words.\n\t\t" << export_shortest_time.count() << " milliseconds.\n\t"
			<< same_letter_words_from_dictionary.size() << " same letter words.\n\t\t" << export_sameletter_time.count() << " milliseconds.\n\t"
			<< most_s_words.size() << " most s (subtype) words.\n\t\t" << export_mostS_time.count() << " milliseconds.\n\t"
			<< most_b_words.size() << " most b (subtype) words.\n\t\t" << export_mostB_time.count() << " milliseconds.\n\t"
			<< random_words_vec.size() << " random words.\n\t\t" << export_random_time.count() << " milliseconds.\n";

		benchmark_results << "14. Serialization.\n\tSaved the dictionary.\n\t" << save_dictionary_time.count() << " milliseconds.\n\t"
			<< "Loaded the dictionary.\n\t" << load_dictionary_time.count() << " milliseconds.\n";

		benchmark_results << std::endl;
	}

	std::cout << "HUGE TEST is done. See benchmark results in " << resfile << std::endl;
}

struct RussianTestSubtype : public dictionary_creator::Entry
{
	RussianTestSubtype(dictionary_creator::utf8_string str = "default", int da = int{}, size_t utflen = size_t{})
		: dictionary_creator::Entry{ std::move(str) }, da{ da }, utflen{ utflen }
	{}
	int da;
	size_t utflen;

	template <typename A>
	void serialize(A &arch, const unsigned int version)
	{
		arch & boost::serialization::base_object<dictionary_creator::Entry>(*this);
		arch & da;
		arch & utflen;
	}
};

BOOST_CLASS_EXPORT_GUID(RussianTestSubtype, typeid(RussianTestSubtype).name())

void huge_test::main_huge_test_russian()
{
	std::cout << u8"\nHUGE TEST for DictionaryManager lib and features. Русская версия.\n" << std::endl;

	auto basic_words = prepare_words(u8"а", u8"я");

	auto first_set = basic_words;
	while (first_set.size() > 1'000'000)
	{
		first_set.erase(first_set.begin());
	}
	auto second_set = basic_words;
	while (second_set.size() > 1'000'000)
	{
		auto it = second_set.end();
		std::advance(it, -1);
		second_set.erase(it);
	}

	auto print_to_file = [] (const std::set<std::string> &set, std::string file_name)
	{
		if (auto output = std::ofstream(file_name); output.good())
		{
			size_t nl = 0;
			for (const auto &i: set)
			{
				output << i << (++nl % 10 ? '\t' : '\n');
			}
			output << std::endl;
		}
		std::cout << "\tSet of " << set.size() << " words is written to " << file_name << std::endl;
	};

	std::string source_1 = "HUGE_TEST_source_file_one.rus.txt";
	std::string source_2 = "HUGE_TEST_source_file_two.rus.txt";

	dictionary_creator::DictionaryManager russian(dictionary_creator::Language::Russian);
	bool perfect;
	//
	// 1. Parsing
	//
	std::cout << "\n1. Parsing\n\n";

	print_to_file(first_set, source_1);
	print_to_file(second_set, source_2);

	russian.add_input_file(source_1);
	russian.add_input_file(source_2);

	auto parse_all_task = [&russian] () { russian.parse_all_pending(); };
	auto parse_all_time = huge_test::execution_time(parse_all_task);
	std::cout << "\tParsing test done\n";
	








#ifdef COMMENT_12356734593574034953094580349580348345_NOT_DEFINED
//		IN CASE OF PARANOIA
//
std::string issue_file_words = "HUGE_TEST_issue_output_words.txt";
std::string issue_file_proper_nouns = "HUGE_TEST_issue_output_proper_nouns.txt";

std::ofstream wstream(issue_file_words);
std::ofstream pnstream(issue_file_proper_nouns);

size_t total_w = 0;
size_t total_pn = 0;

	std::cout << "\n\nWords start from letters:\t";
	for (const auto &[l, ws]: russian.dictionary.dictionary)
	{
		std::cout << l << '\t' << std::flush;
		for (const auto &w: ws)
		{
			++total_w;
			wstream << w->get_word() << (total_w % 10 ? '\t' : '\n');
		}
	}
	std::cout << "\n\t\t\tTOTAL: " << total_w << std::endl;


	std::cout << "\n\nProper nouns start from letters:\t";
	for (const auto &[l, pns]: russian.dictionary.proper_nouns)
	{
		std::cout << l << '\t' << std::flush;
		for (const auto &pn: pns)
		{
			++total_pn;
			pnstream << pn->get_word() << (total_pn % 10 ? '\t' : '\n');
		}
	}
	std::cout << "\n\t\t\tTOTAL: " << total_pn << std::endl;

	size_t total_fs = 0;
	for (const auto &i: first_set)
	{
		if (dictionary_creator::utf8_length(i) >= 3ul)
		{
			++total_fs;
		}
	}
	std::cout << "\nFrom first set of " << first_set.size() << " words, suitable length words in total: " << total_fs << std::endl;

	size_t total_ss = 0;
	for (const auto &i: second_set)
	{
		if (dictionary_creator::utf8_length(i) >= 3ul)
		{
			++total_ss;
		}
	}
	std::cout << "\nFrom first set of " << second_set.size() << " words, suitable length words in total: " << total_ss << std::endl;

	[&first_set, &second_set] ()
	{
		first_set.merge(second_set);
		size_t ttotal = 0;
		for (const auto &i: first_set)
		{
			if (dictionary_creator::utf8_length(i) >= 3UL)
			{
				++ttotal;
			}
		}
		std::cout << "\nTotally of " << first_set.size() << " words, suitable length words: " << ttotal << std::endl;
	}();
#endif

	//
	// 2. Contains check
	//
	std::cout << "\n2. Contains check\n\n";
	perfect = true;
	auto contains_check_task = [&perfect, &russian, &first_set, &second_set]
	{
		auto check = [&perfect, &russian] (const std::set<std::string> &set)
		{
			for (const auto &i: set)
			{
				bool contains = russian.contains_word(i);
				bool goodsize = dictionary_creator::utf8_length(i) >= 3;

				if (contains ^ goodsize)
				{
					std::cout << "\t\tError: " << i << " [" << i.size() << "] is"
						<< (contains ? "" : "n't") << " in the dictionary\n";
					perfect = false;
				}
			}
		};
		check(first_set);
		check(second_set);
	};
	auto contains_check_time = huge_test::execution_time(contains_check_task);

	if (perfect)
	{
		std::cout << "\tWords from both set (" << (first_set.size() + second_set.size())
			<< " in total) are contained in dictionary. Test passed perfectly." << std::endl;
	}
	else
	{
		std::cout << "\tError: some words were missing" << std::endl;
	}
	std::cout << "\tContains check done\n";
	//
	// 3. Letter subset
	//
	std::cout << "\n3. Letter subset\n\n";
	perfect = true;
	auto starting_match_task = [&russian, &perfect] ()
	{
		dictionary_creator::letter_type first_letter = " ";

		for (dictionary_creator::letter_type i = u8"а"; i <= u8"я"; i = get_next_letter(i))
		{
			first_letter = i;
			auto letter_subset = russian.get_subset(first_letter);

			if (letter_subset.empty())
			{
				std::cout << "\t\tEmpty subset starting from letter " << i << '\n';
				perfect = false;
			}

			for (const auto &word: letter_subset)
			{
				if (auto fl = dictionary_creator::first_letter(word->get_word(), dictionary_creator::Language::Russian); fl != i)
				{
					std::cout << "\t\tError: in subset for letter " << first_letter << " there is a word "
							<< word->get_word() << ", starting with " << fl << '\n';
					perfect = false;
				}
			}
		}
	};
	auto starting_match_time = huge_test::execution_time(starting_match_task);
	if (perfect)
	{
		std::cout << "\tReturned letter starting subset perfectly fine\n";
	}
	else
	{
		std::cout << "\tSome letter subset were flawed\n";
	}
	std::cout << "\tLetter subset test done\n";
	//
	// 4. Shortest words subset
	//
	std::cout << "\n4. Shortest words subset\n";
	perfect = true;
	auto under_4 = [&first_set, &second_set] ()
	{
		std::set<std::string> set;
		auto add = [&set] (const std::set<std::string> &s)
		{
			for (auto i: s)
			{
				size_t size = dictionary_creator::utf8_length(i);
				if (2 < size && size < 4)
				{
					set.emplace(std::move(i));
				}
			}
		};
		add(first_set);
		add(second_set);
		return set;
	}().size();
	std::cout << "\t" << under_4 << " words should be < 4 letters long\n";

	dictionary_creator::subset_t shortest_words_subset;
	auto shortest_subset_test = [&russian, &shortest_words_subset, under_4] ()
	{
		shortest_words_subset = russian.get_subset(dictionary_creator::ComparisonType::Shortest, under_4);
	};
	auto shortest_subset_time = huge_test::execution_time(shortest_subset_test);

	if (shortest_words_subset.size() != under_4)
	{
		perfect = false;
		std::cout << "\tError: shortest words subset size = " << shortest_words_subset.size() << u8" \U00002260 "
			<< under_4 << " = requested for that subset words" << std::endl;
	}
	for (const auto &i: shortest_words_subset)
	{
		if (auto size = dictionary_creator::utf8_length(i->get_word()); size >= 4)
		{
			std::cout << "\t\tError: [" << size << "] letter word returned as one of " << under_4 << " shortest words\n";
			perfect = false;
		}
	}
	std::cout << "\tShortest words subset test done " << (perfect ? "perfectly" : "with flaws") << std::endl;
	//
	// 5. Longest words subset
	//
	std::cout << "\n5. Longest words subset\n\n";
	auto third_set = [&first_set, &second_set] (size_t number)
	{
		srand(time(nullptr));

		std::set<std::string> set;

		size_t counter = 0;
		while (set.size() < number)
		{
			auto first_it = first_set.begin();
			std::advance(first_it,
					dictionary_creator::random_number(first_set.size())
	//			//	counter
					);
			auto second_it = second_set.begin();
			std::advance(second_it,
					dictionary_creator::random_number(second_set.size())
	//			//	counter
					);
			++counter;

			std::string newstring = { *first_it + *second_it };

			auto [iter, res] = set.insert(newstring);
			if (!res)
			{
				std::cout << "\t\tError: \"" << newstring << "\" not inserted" << std::endl;
			}
			else
			{
				std::cout << (counter % 100 ? "" : ".") << std::flush;
				if (counter % 10'000 == 0)
				{
					std::cout << " " << counter << " / " << number << std::endl;
				}
			}
		}
		return set;
	}(10'000);

	std::string source_3 = "HUGE_TEST_source_file_three.rus.txt";
	print_to_file(third_set, source_3);
	russian.add_input_file(source_3);
	russian.parse_all_pending();

	perfect = true;
	auto over_5 = std::count_if(third_set.begin(), third_set.end(), [] (const std::string &s) { return dictionary_creator::utf8_length(s) > 5; });
	dictionary_creator::subset_t longest_words_subset;

	auto longest_subset_test = [&russian, &longest_words_subset, over_5]
	{
		longest_words_subset = russian.get_subset(dictionary_creator::ComparisonType::Longest, over_5);
	};
	auto longest_subset_time = huge_test::execution_time(longest_subset_test);

	if (longest_words_subset.size() != over_5)
	{
		perfect = false;
		std::cout << "\tError: longest words subset size = " << longest_words_subset.size() << u8" ≠ "
			<< over_5 << " = requested for longest subset number" << std::endl;
	}
	for (const auto &i: longest_words_subset)
	{
		if (auto size = dictionary_creator::utf8_length(i->get_word()); size <= 5)
		{
			std::cout << "\t\tError: [" << size << "] letter word returned as one of " << over_5 << " longest words\n";
			perfect = false;
		}
	}
	for (auto i: third_set)
	{
		if (!russian.contains_word(i))
		{
			std::cout << "\tError: " << i << " somehow disappeared from the dictionary" << std::endl;
			perfect = false;
		}
	}
	std::cout << "\tLongest words subset done " << (perfect ? "perfectly" : "with flaws") << std::endl;
	//
	// 6. Custom subset
	//
	std::cout << "\n6. Custom subset\n\n";
	auto same_letter_first = [] (const dictionary_creator::Entry &a, const dictionary_creator::Entry &b)
	{
		auto same_letter = [] (const dictionary_creator::utf8_string &word)
		{
			if (word.size() % 2 != 0)
			{
				throw std::runtime_error("Probably a broken russian UTF-8 word");
			}

			std::vector<dictionary_creator::letter_type> letters(word.size() / 2, dictionary_creator::letter_type(2, 0x0));
			for (size_t i = 0; i != word.size();)
			{
				letters[i / 2][0] = word[i];
				letters[i / 2][1] = word[i + 1];

				i += 2;
			}

			for (auto i: letters)
			{
				if (i != letters.front())
				{
					return false;
				}
			}
			return true;
		};

		auto same_a = same_letter(a.get_word());
		auto same_b = same_letter(b.get_word());

		if (same_a && !same_b)
		{
			return true;
		}
		else if (same_b && !same_a)
		{
			return false;
		}
		else
		{
			return a.get_word() < b.get_word();
		}
	};
	auto same_letter = [] (const dictionary_creator::utf8_string &word)
	{
		if (word.size() % 2 != 0)
		{
			throw std::runtime_error("Probably a broken russian UTF-8 word");
		}

		std::vector<dictionary_creator::letter_type> letters(word.size() / 2, dictionary_creator::letter_type(2, 0x0));
		for (size_t i = 0; i != word.size();)
		{
			letters[i / 2][0] = word[i];
			letters[i / 2][1] = word[i + 1];

			i += 2;
		}

		for (auto i: letters)
		{
			if (i != letters.front())
			{
				return false;
			}
		}
		return true;
	};

	std::set<std::string> same_letter_words_from_sets;
	auto populate_same_letter_words_from_sets = [&same_letter, &same_letter_words_from_sets] (const std::set<std::string> &arg_set)
	{
		for (const auto &i: arg_set)
		{
			if (same_letter(i) && dictionary_creator::utf8_length(i) >= 3)
			{
				same_letter_words_from_sets.insert(i);
			}
		}
	};
	populate_same_letter_words_from_sets(first_set);
	populate_same_letter_words_from_sets(second_set);
	populate_same_letter_words_from_sets(third_set);
	auto different_same_letter_words = same_letter_words_from_sets.size();

	std::cout << "\t" << different_same_letter_words << " words have perfect match of each letter" << std::endl;

	dictionary_creator::subset_t same_letter_words_from_dictionary;
	auto custom_compare_task = [&russian, &same_letter_first, &same_letter_words_from_dictionary, different_same_letter_words]
	{
		same_letter_words_from_dictionary = russian.get_subset(same_letter_first, different_same_letter_words);
	};
	auto custom_compare_time = huge_test::execution_time(custom_compare_task);

	perfect = true;
	for (const auto &i: same_letter_words_from_dictionary)
	{
		if (!same_letter(i->get_word()))
		{
			std::cout << "\t\tError: custom comparator top contains word \"" << i->get_word()
				<< "\" that consits from different letters\n";
			perfect = false;
		}
	}
	std::cout << "\tCustom subset test done " << (perfect ? "perfectly" : "with flaws") << std::endl;
	//
	// 7. Add as subtype
	//
	std::cout << "\n7. Add as subtype\n\n";
	std::set<dictionary_creator::utf8_string> fourth_set;
	for (const auto &i: first_set)
	{
		fourth_set.insert(u8"д" + i + u8"а");
	}
	auto da_counter = [] (const dictionary_creator::utf8_string &string)
	{
		size_t da = 0;

		size_t pos = std::string::npos;

		while (true)
		{
			pos = string.rfind(dictionary_creator::utf8_string{ u8"да" }, pos);

			if (pos != std::string::npos)
			{
				++da;
				if (pos == 0)
				{
					break;
				}
				--pos;
			}
			else
			{
				break;
			}
		}

		return da;
	};
	auto add_subtype_task = [&fourth_set, &russian, &da_counter] ()
	{
		for (auto i: fourth_set)
		{
			russian.lookup_or_add_word<RussianTestSubtype>(i,	da_counter(i), dictionary_creator::utf8_length(i));
		}
	};
	auto add_subtype_time = huge_test::execution_time(add_subtype_task);

	perfect = true;
	for (auto i: fourth_set)
	{
		if (!russian.contains_word(i))
		{
			perfect = false;
			std::cout << "\tError: " << i << " isn't in dictionary" << std::endl;
		}
	}
	std::cout << "\tAdd words as Entry subtype done " << (perfect ? "perfectly" : "with flaws") << std::endl;
	//
	// 8-9. Subtype subsets
	//
	auto most_da_first = [] (const RussianTestSubtype &a, const RussianTestSubtype &b)
	{
		return (a.da > b.da) || (a.da == b.da && a.get_word() < b.get_word());
	};

	auto utflen_longest = [] (const RussianTestSubtype &a, const RussianTestSubtype &b)
	{
		return (a.utflen > b.utflen) || (a.utflen == b.utflen && a.get_word() < b.get_word());
	};

	dictionary_creator::subset_t most_da_words;
	auto get_most_da_task = [&russian, &most_da_words, &most_da_first] ()
	{
		most_da_words = russian.get_subset(most_da_first, 500'000);
	};
	auto get_most_da_time = huge_test::execution_time(get_most_da_task);

	std::cout << u8"\n8. Subtype subset: most \'да\'\n\n";
	perfect = true;
	int check_da = dynamic_cast<RussianTestSubtype *>(most_da_words.front().get())->da;
	for (auto i: most_da_words)
	{
		if (auto ab = dynamic_cast<RussianTestSubtype *>(i.get()); ab)
		{
			if (ab->da > check_da)
			{
				std::cout << "\tError: " << ab->get_word() << " [" << ab->da << " > " << check_da << "]" << std::endl;
				perfect = false;
			}
			else if (ab->da < check_da)
			{
				check_da = ab->da;
			}
		}
		else
		{
			std::cout << "\tError: returned pointer won't dynamic cast, must be a wrong pointer" << std::endl;
			perfect = false;
		}
	}
	std::cout << u8"\tSubtype subset: most \'да\' done " << (perfect ? "perfectly" : "with flaws") << std::endl;

	dictionary_creator::subset_t utflen_longest_words;
	auto get_utflen_longest_task = [&russian, &utflen_longest_words, &utflen_longest] ()
	{
		utflen_longest_words = russian.get_subset(utflen_longest, 500'001);
	};
	auto get_utflen_longest_time = huge_test::execution_time(get_utflen_longest_task);

	std::cout << "\n9. Subtype subset: UTF-8 length longest words'\n\n";
	perfect = true;
	double check_len = dynamic_cast<RussianTestSubtype *>(utflen_longest_words.front().get())->utflen;

	for (auto it = utflen_longest_words.cbegin(); it != utflen_longest_words.cend(); ++it)
	{
		if (auto ab = dynamic_cast<RussianTestSubtype *>(it->get()); ab)
		{
			if (ab->utflen > check_len)
			{
				std::cout << "\tError: " << ab->get_word() << " [" << ab->utflen << " > " << check_len << "]" << std::endl;
				perfect = false;
			}
			else if (ab->utflen < check_len)
			{
				check_len = ab->utflen;
			}
		}
		else
		{
			std::cout << "\tError: pointer won't cast, improper custom subtype sorting result" << std::endl;
			perfect = false;
		}
	}
	std::cout << "\tSubtype subset: UTF-8 length longest words done " << (perfect ? "perfectly" : "with flaws") << std::endl;
	//
	// 10. Random words
	//
	std::cout << "\n10. Random words\n\n";
	dictionary_creator::subset_t random_words_vec;
	size_t random_words_number = 1'000'000;
	auto random_words_task = [&russian, &random_words_number, &random_words_vec]
	{
		random_words_vec = russian.get_random_words(random_words_number);
	};
	auto random_words_time = huge_test::execution_time(random_words_task);

	if (auto setsize = std::set<std::shared_ptr<dictionary_creator::Entry>>(random_words_vec.begin(), random_words_vec.end()).size();
		       setsize != random_words_vec.size())
	{
		std::cout << "\tError: some of the random words returned match\t" << setsize << " unique instead of "
			<< random_words_number << std::endl;
	}
	std::cout << "\tRandom words test done" << std::endl;
	//
	// 11. Default export
	//
	std::cout << "\n11. Default export\n\n";
	std::string output_1 = "HUGE_TEST_output_file_1.all_the_dictionary.rus.txt";
	auto default_export_task = [&russian, &output_1]
	{
		if (auto output = std::ofstream(output_1); output.good())
		{
			russian.set_output(output);
			russian.export_dictionary();
		}
	};
	auto default_export_time = huge_test::execution_time(default_export_task);

	if (std::ifstream(output_1).good())
	{
		std::cout << "\tExported all the dictionary in " << output_1 << std::endl;
	}
	std::cout << "\tDefault export done" << std::endl;
	//
	// 12. Export top
	//
	std::cout << "\n12. Export top\n\n";
	russian.add_input_file(output_1);
	russian.parse_all_pending();

	std::string output_2 = "HUGE_TEST_output_file_2.top_of_most_frequent_words.rus.txt";
	auto export_frequent_task = [&russian, &output_2]
	{
		if (auto output = std::ofstream(output_2); output.good())
		{
			russian.set_output(output);
			russian.export_top(dictionary_creator::ComparisonType::MostFrequent, 1'000'000);
		}
	};
	auto export_frequent_time = huge_test::execution_time(export_frequent_task);

	if (std::ifstream test(output_2); test.good())
	{
		std::cout << "\tExported top of most frequent words to " << output_2 << std::endl;
	}
	std::cout << "\tExport top most frequent words test is done" << std::endl;
	//
	// 13. Export subsets
	//
	std::cout << "\n13. Export subsets\n\n";
	std::string output_3 = "HUGE_TEST_output_file_3.subset_of_longest_words.rus.txt";
	auto export_longest_task = [&russian, &longest_words_subset, &output_3]
	{
		if (auto output = std::ofstream(output_3); output.good())
		{
			russian.set_output(output);
			russian.export_subset(longest_words_subset);
		}
	};
	auto export_longest_time = huge_test::execution_time(export_longest_task);

	if (std::ifstream test(output_3); test.good())
	{
		std::cout << "\tExported subset of longest words to " << output_3 << std::endl;
	}

	std::string output_4 = "HUGE_TEST_output_file_4.subset_shortest_w.rus.txt";
	auto export_shortest_task = [&russian, &subset = std::as_const(shortest_words_subset), &file = std::as_const(output_4)]
	{
		if (auto output = std::ofstream(file); output.good())
		{
			output << "\n" << std::endl;
			russian.set_output(output);
			russian.export_subset(subset);
		}
		else
		{
			std::cout << "\tError: cannot export to " << file << std::endl;
		}
	};
	auto export_shortest_time = huge_test::execution_time(export_longest_task);

	if (std::ifstream test(output_4); test.good())
	{
		std::cout << "\tExported subset of shortest words to " << output_4 << std::endl;
	}
	else
	{
		std::cout << "\tHaven't export shortest words subset. Reason is unknown" << std::endl;
	}

	std::string output_5 = "HUGE_TEST_output_file_5.words_of_same_letters.rus.txt";
	auto export_sameletter_task = [&russian, &subset = std::as_const(same_letter_words_from_dictionary), &file = std::as_const(output_5)]
	{
		if (auto output = std::ofstream(file); output.good())
		{
			russian.set_output(output);
			russian.export_subset(subset);
		}
	};
	auto export_sameletter_time = huge_test::execution_time(export_sameletter_task);

	if (std::ifstream test(output_5); test.good())
	{
		std::cout << "\tExported subset of same letter words to " << output_5 << std::endl;
	}

	std::string output_6 = "HUGE_TEST_output_file_6.most_da_first.rus.txt";
	auto export_mostS_task = [&russian, &subset = std::as_const(most_da_words), &file = std::as_const(output_6)]
	{
		if (auto output = std::ofstream(file); output.good())
		{
			russian.set_output(output);
			russian.export_subset(subset);
		}
	};
	auto export_mostS_time = huge_test::execution_time(export_mostS_task);

	if (std::ifstream test(output_6); test.good())
	{
		std::cout << "\tExported subset of most letter \"s\" containing words to " << output_6 << std::endl;
	}

	std::string output_7 = "HUGE_TEST_output_file_7.utflen_longest.rus.txt";
	auto export_mostB_task = [&russian, &subset = std::as_const(utflen_longest_words), &file = std::as_const(output_7)]
	{
		if (auto output = std::ofstream(file); output.good())
		{
			russian.set_output(output);
			russian.export_subset(subset);
		}
	};
	auto export_mostB_time = huge_test::execution_time(export_mostB_task);

	if (std::ifstream test(output_7); test.good())
	{
		std::cout << "\tExported subset of most letter \"b\" dominated words to " << output_7 << std::endl;
	}

	std::string output_8 = "HUGE_TEST_output_file_8.random_words.rus.txt";
	auto export_random_task = [&russian, &subset = std::as_const(random_words_vec), &file = std::as_const(output_8)]
	{
		if (auto output = std::ofstream(file); output.good())
		{
			russian.set_output(output);
			russian.export_subset(subset);
		}
	};
	auto export_random_time = huge_test::execution_time(export_random_task);

	if (std::ifstream test(output_8); test.good())
	{
		std::cout << "\tExported subset of random words to " << output_8 << std::endl;
	}
	//
	// 14. Serialization
	//
	std::cout << "\n14. Serialization\n\n";
	dictionary_creator::utf8_string serialization_filename = "HUGE_TEST_serialization file russian";
	std::cout << "\tBefore rename: " << russian.get_name() << '\n';
	russian.rename(serialization_filename);
	std::cout << "\tAfter rename: " << russian.get_name() << '\n';
	auto save_dictionary_task = [&russian]
	{
		russian.save_dictionary();
	};
	auto save_dictionary_time = huge_test::execution_time(save_dictionary_task);

	dictionary_creator::DictionaryManager russian_read(dictionary_creator::Language::Russian);
	perfect = false;
	auto load_dictionary_task = [&russian_read, &serialization_filename, &perfect]
	{
		for (const auto &name: dictionary_creator::available_dictionaries())
		{
			if (name.human_readable == serialization_filename)
			{
				russian_read = dictionary_creator::load_dictionary(name.full);
				perfect = true;
				break;
			}
		}
		if (perfect == false)
		{
			std::cout << "\tError: failed to find the expected dictionary among the saved ones" << std::endl;
		}
	};
	auto load_dictionary_time = huge_test::execution_time(load_dictionary_task);
	
	perfect = true;
	{
		auto a_top = russian.get_subset(dictionary_creator::ComparisonType::Shortest, 5'000);
		auto b_top = russian_read.get_subset(dictionary_creator::ComparisonType::Shortest, 5'000);

		if (a_top.size() != b_top.size())
		{
			perfect = false;
			std::cout << "\tError: shortest subset mismatch between original and loaded dictionaries ("
				<< b_top.size() << " instead of expected " << a_top.size() << ")" << std::endl;
		}
		else
		{
			for (size_t i = 0; i != a_top.size(); ++i)
			{
				if (a_top[i]->get_word() != b_top[i]->get_word())
				{
					perfect = false;
					std::cout << "\tError: subset mismatch" << std::endl;
				}
			}
		}
	}
	std::cout << "\tSerialization done " << (perfect ? "perfectly" : "with flaws") << std::endl;
	// OUTPUT BENCHMARK RESULTS
	std::string resfile = "HUGE_TEST_benchmark_results.rus.txt";
	std::ofstream benchmark_results(resfile);
	if (benchmark_results.good())
	{
		benchmark_results << "Benchmark results of HUGE TEST for DictionaryManager lib and features\n" << std::endl;

		benchmark_results << "1. Parsing.\n\t2 sets of files 1'000'000 possible words each.\n\t"
			<< parse_all_time.count() << " milliseconds.\n";

		benchmark_results << "2. Contains check.\n\tCheck if it contains all necessary and no unnecessary words.\n\t"
			<< contains_check_time.count() << " milliseconds.\n";

		benchmark_results << "3. Letter subset.\n\tReturned subset for each russian letter and checked it.\n\t"
			<< starting_match_time.count() << " milliseconds.\n";

		benchmark_results << "4. Shortest subset.\n\tReturned and checked subset of " << under_4 << " shortest words.\n\t"
			<< shortest_subset_time.count() << " milliseconds.\n";

		benchmark_results << "5. Longest subset.\n\tReturned and checked subset of " << over_5 << " longest words.\n\t"
			<< longest_subset_time.count() << " milliseconds.\n";

		benchmark_results << "6. Custom subset.\n\tReturned subset of " << different_same_letter_words << " words via custom comparator.\n\t"
			<< custom_compare_time.count() << " milliseconds.\n";

		benchmark_results << "7. Add as subtype.\n\tAdded " << fourth_set.size() << " new words as Entry derrived type pointers.\n\t"
			<< add_subtype_time.count() << " milliseconds.\n";

		benchmark_results << "8. Subtype subset.\n\tRetrieved subtype custom sorted subset of " << most_da_words.size() << u8" most да words.\n\t"
			<< get_most_da_time.count() << " milliseconds.\n";

		benchmark_results << "9. Subtype subset.\n\tRetrieved subtype custom sorted subset of " << utflen_longest_words.size() << " UTF-8 longest.\n\t"
			<< get_utflen_longest_time.count() << " milliseconds.\n";

		benchmark_results << "10. Random subset.\n\tRetrieved subset of " << random_words_number << " random words.\n\t"
			<< random_words_time.count() << " milliseconds.\n";

		benchmark_results << "11. Default export.\n\tExported all the dictionary with default export settings.\n\t"
			<< default_export_time.count() << " milliseconds.\n";

		benchmark_results << "12. Export top.\n\tExported top 1'000'000 most frequent words.\n\t"
			<< export_frequent_time.count() << " milliseconds.\n";

		benchmark_results << "13. Export subset.\n\t"
			<< longest_words_subset.size() << " longest words.\n\t\t" << export_longest_time.count() << " milliseconds.\n\t"
			<< shortest_words_subset.size() << " shortest words.\n\t\t" << export_shortest_time.count() << " milliseconds.\n\t"
			<< same_letter_words_from_dictionary.size() << " same letter words.\n\t\t" << export_sameletter_time.count() << " milliseconds.\n\t"
			<< most_da_words.size() << u8" most да (subtype) words.\n\t\t" << export_mostS_time.count() << " milliseconds.\n\t"
			<< utflen_longest_words.size() << " UTF-longest (subtype) words.\n\t\t" << export_mostB_time.count() << " milliseconds.\n\t"
			<< random_words_vec.size() << " random words.\n\t\t" << export_random_time.count() << " milliseconds.\n";

		benchmark_results << "14. Serialization.\n\tSaved the dictionary.\n\t" << save_dictionary_time.count() << " milliseconds.\n\t"
			<< "Loaded the dictionary.\n\t" << load_dictionary_time.count() << " milliseconds.\n";

		benchmark_results << std::endl;
	}

	std::cout << "HUGE TEST is done. See benchmark results in " << resfile << std::endl;
}

void d3v3l0p1n9()
{
	std::cout << "Developing features...\n";

	std::cout << u8"Русский алфавит:\n\n";

	dictionary_creator::letter_type lc = u8"а";
	dictionary_creator::letter_type uc = u8"А";

	while (lc <= u8"я")
	{
		std::cout << lc << " → " << dictionary_creator::uppercase_letter(lc, dictionary_creator::Language::Russian) << '\t'
			<< uc << " → " << dictionary_creator::uppercase_letter(uc, dictionary_creator::Language::Russian) << '\n';

		if (lc == u8"е")
		{
			std::cout << u8"ё" << " → " << dictionary_creator::uppercase_letter(u8"ё", dictionary_creator::Language::Russian) << '\t'
				<< u8"Ё" << " → " << dictionary_creator::uppercase_letter(u8"Ё", dictionary_creator::Language::Russian) << '\n';
		}

		lc = get_next_letter(lc);
		uc = get_next_letter(uc);
	}
	std::cout << std::endl;

	dictionary_creator::utf8_string russian_text = u8"А будь Владимир гражданином достойным... Ели ёжиков жили завидно и креативно, Лаврентий"
	       u8" меж них очень протестовал: Россия — страна твоя, Ульянушка, фиников хоть ценных Чеслав Шумилов щупал, экивок Юрмалы... Я?!";
	std::string filename = "HUGE_TEST_source_file_X_testing_russian.txt";
	if (std::ofstream output(filename); output.good())
	{
		output << russian_text << std::endl;
	}

	dictionary_creator::DictionaryCreator dc(dictionary_creator::Language::Russian);
	if (std::ifstream input_stream(filename); input_stream.good())
	{
		dc.add_input(std::move(input_stream));
	}

	auto russian = dc.parse_to_dictionary();
	std::cout << "\nGot this dictionary: " << typeid(russian).name() << std::endl;

	std::cout << "Total words: " << russian.total_words() << std::endl;

	auto checkword = [&russian] (dictionary_creator::utf8_string word)
	{
		if (auto res = russian.lookup(word); res.get() == nullptr)
		{
			std::cout << word << " is absent\n";
		}
	};

	checkword(u8"будь");
	checkword(u8"вы");
	checkword(u8"гад");
	checkword(u8"жить");
	checkword(u8"креативно");
	checkword(u8"и");
	checkword(u8"очень");
	checkword(u8"просто");
	checkword(u8"сука");
	checkword(u8"ценишь");

	std::cout << "\n\n\nAvailable words:\n";
	for (auto i: russian.get_undefined())
	{
		std::cout << i->get_word() << "\n";
	}
	std::cout << std::endl;
}

void huge_test::run_all_tests()
{
	huge_test::special_cases();

//	huge_test::main_huge_test_english();

//	huge_test::main_huge_test_russian();

//	d3v3l0p1n9();
}

// lines 416 and 1249 argument to adjust time (half a million is good but very long)
