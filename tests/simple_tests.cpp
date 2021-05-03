#include "simple_tests.h"

#include <chrono>
#include <thread>

bool simple_tests::isprintable(char character) noexcept
{
	return std::isprint(static_cast<unsigned char>(character));
}


void simple_tests::tests(const char *program_name)
{
	std::cout << "Tests for " << program_name << std::endl;

	simple_tests::test_connections();
/*
	simple_tests::test_entry();

	simple_tests::test_dictionary();

	simple_tests::test_dictionary_creator();

	simple_tests::test_dictionary_exporter();

	simple_tests::test_dictionary_manager();

	simple_tests::test_custom_entry_type();

	simple_tests::test_letter_related_features();

	simple_tests::test_one_line_parser();

	simple_tests::test_serialization();
*/
}

void simple_tests::test_connections()
{
	std::cout << "Testing connections\n";

	auto http = connections::get("http://www.google.com");
	std::cout << "http: " << (http.size() > 100 ? "successfull" : http.data()) << std::endl;
	
	auto https = connections::get("https://api.dictionaryapi.dev/api/v2/entries/en/hello");
	std::cout << "https: " << (https.size() > 100 ? "successfull" : https.data()) << std::endl;

	std::cout << "\n\n" << https << "\n\n";

	auto nonexistent = connections::get("https://nonexis.tent.pa.ge.com");
	std::cout << "nonexistent page: " << nonexistent << std::endl;

	auto notfound = connections::get("https://google.com/pagecan/notbe.found");
	std::cout << "not found: " << notfound << std::endl;

	std::cout << "\n";
}

void utf8_string_as_chars(dictionary_creator::utf8_string str)
{
	std::cout << "\t\t\t" << str << "\n\t\t\t";
	for (size_t i = 0; i != str.size(); ++i)
	{
		std::cout << i << "\t";
	}
	std::cout << "\n\t\t\t";;
	for (size_t i = 0; i != str.size(); ++i)
	{
		if (simple_tests::isprintable(str[i]))
		{
			std::cout << str[i] << "\t";
		}
		else
		{
			std::cout << "{ " << static_cast<int>(str[i]) << " }\t";
		}
	}
	std::cout << std::endl;
}

void simple_tests::test_entry()
{
	std::cout << "Testing Entry class\n";

	dictionary_creator::definer_t definer = [] (dictionary_creator::utf8_string word) -> dictionary_creator::definitions_t
	{
		dictionary_creator::definitions_t definitions;
		definitions[u8"Noun"].emplace(u8"Definition as a noun");
		definitions[u8"Noun"].emplace(u8"Another one definition as a noun");
		definitions[u8"Verb"].emplace(u8"Definition as a verb");

		return definitions;
	};

	dictionary_creator::Entry entry(dictionary_creator::utf8_string{ "Elephant" });

	std::cout << "\t.get_word() " << entry.get_word() << "\n\t.is_defined() " << std::boolalpha << entry.is_defined()
		<< "\n\t.get_counter()\t" << entry.get_counter() << "\n\tstd::cout << entry\t" << entry << std::endl;

	entry.define(definer);

	std::cout << "After it's defined via .define()\n\t.is_defined() " << std::boolalpha << entry.is_defined()
		<< "\n\t.get_counter()\t" << entry.get_counter() << "\n\tstd::cout << entry\t" << entry << std::endl;

	entry.increment_counter();
	std::cout << "After counter incremented via .increment_counter()\n\t.get_counter() " << entry.get_counter() << std::endl;

	for (const auto &[p, ds]: entry.get_definitions())
	{
		for (const auto &d: ds)
		{
			std::cout << p << ": " << d << std::endl;
		}
	}

	dictionary_creator::Entry another(dictionary_creator::utf8_string{ u8"Français" });
	
	std::cout << another << " < " << entry << " : " << (another < entry) << "\t\t"
		<< entry << " < " << another << " : " << (entry < another) << std::endl;

	std::cout << "\n";
}

void simple_tests::test_dictionary()
{
	std::cout << "Testing Dictionary class\n";

	dictionary_creator::Dictionary eng(dictionary_creator::Language::English);
	std::cout << "Type of dictionary is " << typeid(eng).name() << "\tsize is " << sizeof(eng) << std::endl;

	dictionary_creator::utf8_string word = u8"naïve";
	auto first_letter = eng.get_first_letter(word);
	std::cout << "According to the dictionary, first letter of \"" << word << "\" is \'" << first_letter << "\'\n";

	eng.add_word(u8"Naïve");
	eng.add_word("Elephant");
	std::cout << "After 2 .add_word(...) calls\n\t.total_words() is " << eng.total_words() << std::endl;

	std::cout << "\t.lookup(\"elephant\") yields ";
	if (auto ele = eng.lookup("elephant"); ele)
	{
		std::cout << *ele << std::endl;
	}
	else
	{
		std::cout << "NULL" << std::endl;
	}
	std::cout << "\t.lookup(\"Elephant\") yields ";
	if (auto ele = eng.lookup("Elephant"); ele)
	{
		std::cout << *ele << std::endl;
	}
	else
	{
		std::cout << "NULL" << std::endl;
	}

	auto print_dictionary = [] (const auto &d)
	{
		for (const auto &i : d)
		{
			std::cout << "\t" << *i << "\t[" << i->get_counter() << "]" << std::endl;
		}
	};

	auto top_500_longest = eng.get_top(dictionary_creator::ComparisonType::Longest, 500);
	std::cout << "Top 500 longest words according to .get_top(dictionary_creator::ComparisonType::Longest, 500):\n";
	print_dictionary(top_500_longest);

	eng.remove_word("Elephant");
	std::cout << "After we .remove_word(\"Elephant\") it's:\n";
	top_500_longest = eng.get_top(dictionary_creator::ComparisonType::Longest, 500);
	print_dictionary(eng.get_top(dictionary_creator::ComparisonType::Longest, 500));

	eng.add_proper_noun(u8"Naïve");
	std::cout << u8"After .add_proper_noun(u8\"Naïve\") total words is still " << eng.total_words() << std::endl;
	
	std::cout << "Now another call for .get_top(Longest, 500) yields:\n";
	top_500_longest = eng.get_top(dictionary_creator::ComparisonType::Longest, 500);
	print_dictionary(eng.get_top(dictionary_creator::ComparisonType::Longest, 500));

	eng.add_word("eagle");
	eng.add_word("elephant");
	eng.add_word("frog");
	eng.add_word("crow");
	
	dictionary_creator::Dictionary birds(dictionary_creator::Language::English);
	std::cout << "\t.add_word(\"eagle\") first time yields " << birds.add_word("eagle") << std::endl;
	std::cout << "\t.add_word(\"eagle\") second time yields " << birds.add_word("eagle") << std::endl;
	std::cout << "\t.add_word(\"eagle\") third time yields " << birds.add_word("eagle") << std::endl;

	std::cout << "\t.remove_word(\"eagle\") first time yields " << birds.remove_word("eagle") << std::endl;
	std::cout << "\t.remove_word(\"eagle\") second time yields " << birds.remove_word("eagle") << std::endl;
	std::cout << "\t.remove_word(\"eagle\") third time yields " << birds.remove_word("eagle") << std::endl;

	birds.add_word("eagle");
	birds.add_word("crow");
	birds.add_word("cassowary");
	birds.add_word("tit");

	std::cout << "First dictionary:\n";
	print_dictionary(eng.get_top(dictionary_creator::ComparisonType::Longest, 50));
	std::cout << "Second dictionary:\n";
	print_dictionary(birds.get_top(dictionary_creator::ComparisonType::Longest, 50));

	auto sum = eng + birds;
	std::cout << "First + Second = Sum dictionary:\n";
	print_dictionary(sum.get_top(dictionary_creator::ComparisonType::Longest, 50));

	dictionary_creator::Dictionary more_birds(dictionary_creator::Language::English);
	more_birds.add_word("swallow");
	more_birds.add_word("seagull");
	more_birds.add_word("penguin");

	std::cout << "Birds dictionary:\n";
	print_dictionary(birds.get_top(dictionary_creator::ComparisonType::Longest, 50));
	std::cout << "More birds dictionary:\n";
	print_dictionary(more_birds.get_top(dictionary_creator::ComparisonType::Longest, 50));

	birds += more_birds;

	std::cout << "After birds += more birds\n";
	std::cout << "Birds dictionary:\n";
	print_dictionary(birds.get_top(dictionary_creator::ComparisonType::Longest, 50));
	std::cout << "More birds dictionary:\n";
	print_dictionary(more_birds.get_top(dictionary_creator::ComparisonType::Longest, 50));

	birds.remove_word("crow");
	birds.remove_word("seagull");
	birds.remove_word("penguin");

	std::cout << "Attempt to add mismatching languages dictionaries\n";
	dictionary_creator::Dictionary russian(dictionary_creator::Language::Russian);
	try
	{
		eng + russian;
	}
	catch (std::exception &e)
	{
		std::cerr << "An exception caught: " << e.what() << std::endl;
	}

	std::cout << "First:\n";
	print_dictionary(eng.get_top(dictionary_creator::ComparisonType::Longest, 50));
	std::cout << "Second:\n";
	print_dictionary(birds.get_top(dictionary_creator::ComparisonType::Longest, 50));
	std::cout << "Third:\n";
	print_dictionary(more_birds.get_top(dictionary_creator::ComparisonType::Longest, 50));
	
	dictionary_creator::Dictionary beasts(dictionary_creator::Language::English);
	beasts.add_word(u8"bear");
	beasts.add_word("lion");
	beasts.add_word("hamster");
	beasts.add_proper_noun("sin");
	std::cout << "Fourth:\n";
	print_dictionary(beasts.get_top(dictionary_creator::ComparisonType::Longest, 50));

	auto cpp17_sum = arbitrary_sum(eng, birds, more_birds, beasts);
	std::cout << "Sum done as (arg + ...) of all beforementioned:\n";
	print_dictionary(cpp17_sum.get_top(dictionary_creator::ComparisonType::Longest, 50));
	
	auto get_dictionary = []()
	{
		dictionary_creator::Dictionary temp(dictionary_creator::Language::English);
		temp.add_word("archbishop");
		temp.add_word("atheist");
		temp.add_word("adversary");
		temp.add_word("antidiestablishmentarianism");
		temp.add_word("priest");
		temp.add_word("nun");
		temp.add_word("sin");
		temp.add_word("lion");
		temp.add_word("bear");
		temp.add_proper_noun("hamster");
		return temp;
	};

	std::cout << "Now merge with some rval dictionary objects:\n";
	auto cpp17_sum_with_rvals = arbitrary_sum(birds, get_dictionary(), get_dictionary(), get_dictionary());
	print_dictionary(cpp17_sum_with_rvals.get_top(dictionary_creator::ComparisonType::Longest, 50));

	beasts += get_dictionary();
	std::cout << "Beasts after += of temporary rvalue:\n";
	print_dictionary(beasts.get_top(dictionary_creator::ComparisonType::Longest, 50));

	std::cout << "Tricky case: beasts.merge(birds + more_birds):\n";
	beasts.merge(birds + more_birds);
	print_dictionary(beasts.get_top(dictionary_creator::ComparisonType::Longest, 50));

	std::cout << "Dictionary that we are going to subtract from there:\n";
	birds.add_proper_noun("adversary");
	print_dictionary(birds.get_top(dictionary_creator::ComparisonType::Longest, 50));

	beasts.subtract(birds);
	std::cout << "Result of subraction:\n";
	print_dictionary(beasts.get_top(dictionary_creator::ComparisonType::Longest, 50));

	std::cout << "And after -= arbitrary_sum(...):\n";
	beasts -= arbitrary_sum(birds, more_birds, eng, get_dictionary());
	print_dictionary(beasts.get_top(dictionary_creator::ComparisonType::Longest, 50));

	std::cout << "After add \"adversary\", \"hamster\", and \"sin\" which were mentioned as proper nouns\n"
		<< "\tand \"beat\", \"ghost\", \"deer\" which weren't:\n";
	beasts.add_word("adversary");
	beasts.add_word("hamster");
	beasts.add_word("sin");
	beasts.add_word("beat");
	beasts.add_word("ghost");
	beasts.add_word("deer");
	print_dictionary(beasts.get_top(dictionary_creator::ComparisonType::Longest, 50));

	dictionary_creator::Dictionary music(dictionary_creator::Language::English);
	music.add_word("rock");
	music.add_word("star");
	music.add_word("guitar");
	music.add_word("studio");

	dictionary_creator::Dictionary objects(dictionary_creator::Language::English);
	objects.add_word("rock");
	objects.add_word("planet");
	objects.add_word("star");
	objects.add_word("atom");

	std::cout << "Music dictionary:\n";
	print_dictionary(music.get_top(dictionary_creator::ComparisonType::Longest, 50));
	std::cout << "Objects dictionary:\n";
	print_dictionary(objects.get_top(dictionary_creator::ComparisonType::Longest, 50));
	std::cout << "Their intersection:\n";
	print_dictionary(music.intersection_with(objects).get_top(dictionary_creator::ComparisonType::Longest, 50));

	music *= objects - beasts;
	std::cout << "Music after it's *= objects - beasts:\n";
	print_dictionary(music.get_top(dictionary_creator::ComparisonType::Longest, 50));
		
	std::cout << "\n";
}

void simple_tests::test_dictionary_creator()
{
	std::cout << "Testing DictionaryCreator class\n";

	dictionary_creator::DictionaryCreator creator(dictionary_creator::Language::English);
	
	std::cout << "DictionaryCreator type " << typeid(creator).name() << "\t\tsize " << sizeof(creator) << std::endl;

	dictionary_creator::utf8_string file_name{ "English_Q.text" };
	std::unique_ptr<std::istream> uptr_ifs{ new std::ifstream(file_name) };
	creator.add_input(std::move(uptr_ifs));

	std::cout << "Parsing file " << file_name << " (sure if it exists)\n";
	
	auto print_dictionary = [](const auto &d)
	{
		for (const auto &i : d)
		{
			std::cout << "\t" << *i << "\t[" << i->get_counter() << "]" << std::endl;
		}
	};
	
	dictionary_creator::utf8_string another_file_name{ "English_A.text" };
	std::unique_ptr<std::istream> uptr_ifs2{ new std::ifstream(another_file_name) };
	creator.add_input(std::move(uptr_ifs2));
	std::cout << "Also we add another file that is " << another_file_name << " the dictionary is:\n";
	print_dictionary(creator.parse_to_dictionary().get_top(dictionary_creator::ComparisonType::MostFrequent, 1000));
}

void simple_tests::test_dictionary_exporter()
{
	std::cout << "Testing DictionaryExporter class\n";

	dictionary_creator::Dictionary eng(dictionary_creator::Language::English);

	eng.add_word("power");
	eng.add_word("money");
	eng.add_word("money");
	eng.add_word("power");
	eng.add_word("minute");
	eng.add_word("after");
	eng.add_word("minute");
	eng.add_word("hour");
	eng.add_word("after");
	eng.add_word("hour");
	eng.add_word("think");

	auto print_dictionary = [](const auto &d)
	{
		for (const auto &i : d)
		{
			std::cout << "\t" << *i << "\t[" << i->get_counter() << "]" << std::endl;
		}
	};

	std::cout << "Dictionary to export:\n";
	print_dictionary(eng.get_top(dictionary_creator::ComparisonType::LeastFrequent, 50));

	std::cout << "If the stream passed to exporter as an argument isn't in a good state:\n";
	try
	{
		std::cout.setstate(std::ios::badbit);
		std::cerr << "cout.good(): " << std::boolalpha << std::cout.good() << std::endl;
		dictionary_creator::DictionaryExporter must_fail(&std::cout);
	}
	catch (std::exception &e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	std::cout.clear();

	dictionary_creator::DictionaryExporter exporter(&std::cout);
	std::cout << "DictionaryExporter type " << typeid(exporter).name() << "\tsize " << sizeof(exporter) << std::endl;

	std::cout << "Now using it to export given dictionary to std::cout:\n";
	exporter.export_dictionary(eng);

	auto definer = [] (dictionary_creator::utf8_string)
	{
		return dictionary_creator::definitions_t{ { "Noun", { "Some object", "Something" } }, { "Adjective", { "Good" } } };
	};

	auto entries = eng.get_top(dictionary_creator::ComparisonType::Shortest, 50);
	for (const auto &i : entries)
	{
		i->define(definer);
	}
	std::cout << "Exporting again, now after they're defined\n";
	exporter.export_dictionary(eng);
}

void simple_tests::test_dictionary_manager()
{
	std::cout << "Testing DictionaryManager class\n";

	dictionary_creator::DictionaryManager dm(dictionary_creator::Language::English);
	std::cout << "DictionaryManager type " << typeid(dm).name() << "\tsize " << sizeof(dm) << std::endl;

	std::cout << "\n\t\t\t\tI. add_input_file and parse_all_pending\n";
	{
		dictionary_creator::DictionaryManager copy(dictionary_creator::Language::English);
		std::string file_name_one = "English_Q.text";
		std::string file_name_two = "English_A.text";
		std::ifstream english_Q{ file_name_one };

		std::cout << "Using files:\n\t" << file_name_one << "\n\t" << file_name_two << std::endl;
		copy.add_input_file(file_name_one);
		copy.add_input_file(file_name_two);
		copy.parse_all_pending();

		std::string out_file{ "TEST_QandA.text" };
		std::ofstream out_stream(out_file);
		copy.set_output(out_stream);
		copy.export_dictionary();
		std::cout << "See output results in " << out_file << std::endl;
	}

	std::cout << "\n\t\t\t\tII. .lookup_or_add_word and .get of different flavours\n";
	if (auto mentry = dm.lookup_or_add_word("masterpiece"); mentry)
	{
		std::cout << "\t.lookup_or_add_word(\"masterpiece\") returns " << typeid(mentry).name()
			<< " that has word " << *mentry << std::endl;
	}
	else
	{
		std::cout << "\t.lookup_or_add_word(...) returned nullptr which should've been practically impossible\n";
		return;
	}

	std::cout << "\t.contains_word(\"masterpiece\") yields " << std::boolalpha << dm.contains_word("masterpiece")
		<< "\t.contains_word(\"caravan\") yields " << std::boolalpha << dm.contains_word("caravan") << std::endl;

	if (dm.contains_word("masterpiece") == false || dm.contains_word("caravan") == true)
	{
		std::cout << "\t.contains_word(...) must be wrong\n";
		return;
	}

	std::vector<dictionary_creator::utf8_string> words
	{
		"amen", "birth", "cat", "defenestration", "encapsulation", "firefighter", "global", "hunting", "incunabula", "jolly"
	};

	for (const auto &i: words)
	{
		dm.lookup_or_add_word(i);
	}

	auto three_shortest = dm.get_subset(dictionary_creator::ComparisonType::Shortest, 3ul);
	if (three_shortest.size() == 3 
		&& three_shortest[0]->get_word() == dictionary_creator::utf8_string{ "cat" }
		&& three_shortest[1]->get_word() == dictionary_creator::utf8_string{ "amen" })
	{
		std::cout << "Third shortest word is " << three_shortest[2]->get_word() << std::endl;
	}
	else
	{
		std::cout << ".get_subset() returns " << typeid(three_shortest).name() << " with "
			<< three_shortest.size() << " entries inside:\n";

		for (const auto &i: three_shortest)
		{
			std::cout << "\t" << i->get_word() << std::endl;
		}

		std::cout << "\t.get_subset(...) returned inadequate result\n";
		return;
	}

	std::cout << "Subset starting from letter F:\t";
	for (auto i: dm.get_subset("F"))
	{
		std::cout << '\t' << i->get_word() << std::endl;
	}

	std::cout << "Subset starting from letter i:\t";
	for (auto i: dm.get_subset("i"))
	{
		std::cout << '\t' << i->get_word() << std::endl;
	}
	std::cout << std::endl;

	std::cout << "\t.get_undefined(5) yields:\n\t";
	for (auto i: dm.get_undefined(5))
	{
		std::cout << "\t" << i->get_word();
	}
	std::cout << "\n\t.get_undefined() yields:\n\t";
	for (auto i: dm.get_undefined())
	{
		std::cout << "\t" << i->get_word();
	}
	std::cout << std::endl;

	const auto rw_one = dm.get_random_word();
	const auto rw_two = dm.get_random_word();
	if (rw_one && rw_two)
	{
		std::cout << "\t.get_random_word() twice in a row yields:\n\t\t\""
			<< rw_one->get_word() << "\"\tand\t\"" << rw_two->get_word() << "\"" << std::endl;
	}
	else
	{
		std::cout << "\t.get_random_word() must be flawed\n";
		return;
	}

	std::cout << "\t.get_random_words(7) yields:\n\t";
	auto random_7 = dm.get_random_words(7);
	for (const auto i: random_7)
	{
		std::cout << "\t" << i->get_word();
	}
	std::cout << std::endl;

	std::cout << "\n\t\t\t\tIII. .define\n";

	if (auto ff = dm.define("firefighter"); ff && ff->is_defined())
	{
		std::cout << "\t.define(\"firefighter\") yields " << ff->get_word() << " is defined now\n";
	}
	else
	{
		std::cout << "\t.define(...) won't work\n";
		return;
	}

	auto definer = [] (dictionary_creator::utf8_string)
	{
		return dictionary_creator::definitions_t{ { "Noun", { "Some object" } }, { "Adjective", { "Good", "Proper" } } };
	};

	if (auto k = dm.lookup_or_add_word("kite")->define(definer); k.is_defined())
	{
		std::cout << "\t.define() makes " << k.get_word() << " defined\n";
	}
	else
	{
		std::cout << "\t.define() malfunctioning\n";
		return;
	}

	const auto &longest_word = *dm.get_subset(dictionary_creator::ComparisonType::Longest, 1).front();
	if (auto l = dm.define(longest_word); l && l->is_defined())
	{
		std::cout << "\t.define() for longest word " << longest_word.get_word() << " worked\n";
		std::cout << "(also this is " << l->get_word() << ")\n";
	}
	else
	{
		std::cout << "\t.define() isn't working properly\n";
		return;
	}

	std::cout << "Less complex and not so edgy use of .define() on not temporary anymore subset:\n";
	auto shortest_4 = dm.define(dm.get_subset(dictionary_creator::ComparisonType::Shortest, 4));
	for (auto i: shortest_4)
	{
		std::cout << "\t" << i->get_word();
	}
	std::cout << std::endl;

	std::cout << "\n\t\t\t\tIV. .set_output and .export_ features\n";

	std::string output_file_name{ "TEST_output.text" };
	std::ofstream output_stream(output_file_name);
	output_stream.imbue(std::locale("en_US.UTF-8"));
	dm.set_output(output_stream);

	dm.export_dictionary(dictionary_creator::default_export_options | dictionary_creator::ExportOptions::BasicDecorations);
	std::cout << ".export_dictionary(default_export_options | BasicDecorations) → " << output_file_name << std::endl;

	std::string output_file_top{ "TEST_Top_6_longest_defined_only.text" };
	std::ofstream output_file_top_stream{ output_file_top };
	output_file_top_stream.imbue(std::locale("en_US.UTF-8"));

	dm.set_output(output_file_top_stream);
	dm.export_top(dictionary_creator::ComparisonType::Longest, 6,
			dictionary_creator::ExportOptions::DefinedWords 
			| dictionary_creator::ExportOptions::OnlyOneDefinition 
			| dictionary_creator::ExportOptions::BasicDecorations);
	std::cout << ".export_top(Longest, 6, Defined | OnlyOneDefinition | BasicDecorations) → " << output_file_top << std::endl;

	std::string output_for_undefined{ "TEST_undefined_output.text" };
	std::ofstream output_for_undefined_stream{ output_for_undefined };
	dm.set_output(output_for_undefined_stream);
	dm.export_subset(dm.get_undefined(3));
	std::cout << ".export_subset(.get_undefined(3)) → " << output_for_undefined << std::endl;

	std::cout << "If you got this far, then it mustn't be so bad after all. Now go see those output files, really.\n";
}

class DifficultyEntry : public dictionary_creator::Entry
{
public:
	enum class difficulty{ Easy, Normal, Hard, VeryHard };
	DifficultyEntry(dictionary_creator::utf8_string str, difficulty dif) : Entry{ std::move(str) }, word_difficulty{ dif }
	{}
	difficulty get_difficulty() const noexcept
	{
		return word_difficulty;
	}
private:
	difficulty word_difficulty;
};

class ImproperEntry
{
public:
	ImproperEntry(dictionary_creator::utf8_string str) : data{ str }
	{}
private:
	const dictionary_creator::utf8_string data;
};

void simple_tests::test_custom_entry_type()
{
	std::cout << "Testing custom Entry type\n";

	dictionary_creator::DictionaryManager eng(dictionary_creator::Language::English);
/*
	eng.add_input_file("English_A.text");
*/	
	eng.parse_all_pending();

	eng.lookup_or_add_word("antidisestablishmentarianism");

	eng.lookup_or_add_word<DifficultyEntry>("analogy", DifficultyEntry::difficulty::Normal);
	//eng.lookup_or_add_word<ImproperEntry>("improper", DifficultyEntry::difficulty::Normal);
	eng.lookup_or_add_word<DifficultyEntry>("ambiguous", DifficultyEntry::difficulty::Normal);

	auto check_difficulty = [] (const std::shared_ptr<dictionary_creator::Entry> &sp)
	{
		auto de_ptr = dynamic_cast<DifficultyEntry *>(sp.get());
		if (de_ptr)
		{
			std::cout << "Difficulty of \"" << sp->get_word() << "\" is "
				<< static_cast<int>(de_ptr->get_difficulty()) << std::endl;
		}
		else
		{
			std::cout << "Word \"" << sp->get_word() << "\" isn't stored as a DifficultyEntry\n";
		}
	};

	std::cout << "\nChecking difficulty of some entries:\n";
	auto longest = eng.get_subset(dictionary_creator::ComparisonType::Longest, 4);
	for (auto i: longest)
	{
		check_difficulty(i);
	}

	eng.lookup_or_add_word<DifficultyEntry>("analogical", DifficultyEntry::difficulty::Normal);
	eng.lookup_or_add_word<DifficultyEntry>("antidisestablishmentarianism", DifficultyEntry::difficulty::VeryHard);

	std::cout << "\nChecking difficulty again after some updates:\n";
	longest = eng.get_subset(dictionary_creator::ComparisonType::Longest, 4);
	for (auto i: longest)
	{
		check_difficulty(i);
	}

	eng.lookup_or_add_word<DifficultyEntry>("patriotical", DifficultyEntry::difficulty::Normal);
	eng.lookup_or_add_word<DifficultyEntry>("cow", DifficultyEntry::difficulty::Easy);
	eng.lookup_or_add_word<DifficultyEntry>("penultimate", DifficultyEntry::difficulty::Normal);
	eng.lookup_or_add_word<DifficultyEntry>("antepenultimate", DifficultyEntry::difficulty::Hard);

	std::cout << "\nAcquiring subest of 7 most difficult words (lambda):\n";
	auto comp_most_dif = [](const DifficultyEntry &a, const DifficultyEntry &b)
	{
		return static_cast<int>(a.get_difficulty()) > static_cast<int>(b.get_difficulty());
	};
	
	auto most_difficult = eng.get_subset(comp_most_dif, 7);
	for (auto i: most_difficult)
	{
		check_difficulty(i);
	}

	std::cout << "\nAcquiring subest of 10 least difficult words (function pointer):\n";
	auto comp_least_dif = +[](const DifficultyEntry &a, const DifficultyEntry &b)
	{
		return static_cast<int>(a.get_difficulty()) < static_cast<int>(b.get_difficulty());
	};
	
	auto least_difficult = eng.get_subset(comp_least_dif, 10);
	for (auto i: least_difficult)
	{
		check_difficulty(i);
	}
}

void simple_tests::test_letter_related_features()
{
	std::cout << "Testing letter related features: first_letter and uppercase_letter\n";

	auto first_byte = [] (char c) -> bool
	{
		return ((c & 0xC0) != 0x80);
	};

	std::cout << "Example: english\n";
	dictionary_creator::utf8_string eng_lo{ u8"abcdefghijklmnopqrstuvwxyz" };
	dictionary_creator::utf8_string eng_up{ u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
	for (size_t i = 0; i != eng_lo.size(); ++i)
	{
		dictionary_creator::letter_type l(1, eng_lo[i]);
		dictionary_creator::letter_type u(1, eng_up[i]);
		std::cout << '\t' << l << u8" → " << dictionary_creator::uppercase_letter(l, dictionary_creator::Language::English)
			 << '\t' << u << u8" → " << dictionary_creator::uppercase_letter(u, dictionary_creator::Language::English) << std::endl;
	}
	for (auto i: std::set<dictionary_creator::utf8_string>({ "Another", "bright", "idea", "I", "got", "to", "Test", "Features", "1one" }))
	{
		std::cout << "\t[" << dictionary_creator::first_letter(i, dictionary_creator::Language::English) << "] " << i << std::endl;
	}

	std::cout << "First: french\n";
	dictionary_creator::utf8_string fra_lo{ u8"aàâæbcçdeéêèëfghiïîjklmnoôœpqrstuùûüvwxyÿz" };
	dictionary_creator::utf8_string fra_up{ u8"AÀÂÆBCÇDEÉÊÈËFGHIÏÎJKLMNOÔŒPQRSTUÙÛÜVWXYŸZ" };
	for (size_t i = 0; i != fra_lo.size(); ++i)
	{
		if (!first_byte(fra_lo[i]))
		{
			continue;
		}
		dictionary_creator::letter_type l(1, fra_lo[i]);
		if (auto f = fra_lo[i + 1]; !first_byte(f))
		{
			l += f;
		}
		dictionary_creator::letter_type u(1, fra_up[i]);
		if (auto f = fra_up[i + 1]; !first_byte(f))
		{
			u += f;
		}
		std::cout << '\t' << l << u8" → " << dictionary_creator::uppercase_letter(l, dictionary_creator::Language::French)
			 << '\t' << u << u8" → " << dictionary_creator::uppercase_letter(u, dictionary_creator::Language::French) << std::endl;
	}
	for (auto i: std::set<dictionary_creator::utf8_string>({ "Moi", "Ô", "peut", "être", "œufs", "sœurs", "mêtre", "ça", "Æschne" }))
	{
		std::cout << "\t[" << dictionary_creator::first_letter(i, dictionary_creator::Language::French) << "] " << i << std::endl;
	}

	std::cout << "Second: russian\n";
	dictionary_creator::utf8_string rus_lo{ u8"абвгдеёжзийклмнопрстуфхцчшщьыъэюя" };
	dictionary_creator::utf8_string rus_up{ u8"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЬЫЪЭЮЯ" };
	for (size_t i = 0; i != rus_lo.size(); ++i)
	{
		if (!first_byte(rus_lo[i]))
		{
			continue;
		}
		dictionary_creator::letter_type l{ rus_lo[i], rus_lo[i + 1] };
		dictionary_creator::letter_type u{ rus_up[i], rus_up[i + 1] };
		std::cout << '\t' << l << u8" → " << dictionary_creator::uppercase_letter(l, dictionary_creator::Language::Russian)
			 << '\t' << u << u8" → " << dictionary_creator::uppercase_letter(u, dictionary_creator::Language::Russian) << std::endl;
	}
	for (auto i: std::set<dictionary_creator::utf8_string>({ "Обычный", "русский", "текст", "Для", "Проверки", "а", "Я", "не", "сомневался" }))
	{
		std::cout << "\t[" << dictionary_creator::first_letter(i, dictionary_creator::Language::Russian) << "] " << i << std::endl;
	}

	std::cout << "Third: german\n";
	dictionary_creator::utf8_string ger_lo{ u8"aäbcdefghijklmnoöpqrstuüvwxyz" };
	dictionary_creator::utf8_string ger_up{ u8"AÄBCDEFGHIJKLMNOÖPQRSTUÜVWXYZ" };
	for (size_t i = 0; i != ger_lo.size(); ++i)
	{
		if (!first_byte(ger_lo[i]))
		{
			continue;
		}
		dictionary_creator::letter_type l(1, ger_lo[i]);
		if (auto f = ger_lo[i + 1]; !first_byte(f))
		{
			l += f;
		}
		dictionary_creator::letter_type u(1, ger_up[i]);
		if (auto f = ger_up[i + 1]; !first_byte(f))
		{
			u += f;
		}
		std::cout << '\t' << l << u8" → " << dictionary_creator::uppercase_letter(l, dictionary_creator::Language::German)
			 << '\t' << u << u8" → " << dictionary_creator::uppercase_letter(u, dictionary_creator::Language::German) << std::endl;

		if (l == "s")
		{
			l = u8"ß";
			u = u8"ẞ";
			std::cout << '\t' << l << u8" → " << dictionary_creator::uppercase_letter(l, dictionary_creator::Language::German)
				 << '\t' << u << u8" → " << dictionary_creator::uppercase_letter(u, dictionary_creator::Language::German) << std::endl;
		}
	}
	for (auto i: std::set<dictionary_creator::utf8_string>({ "Nun", "liebe", "kinder", "die", "Ärzte", "Scheiße", "Ördner" }))
	{
		std::cout << "\t[" << dictionary_creator::first_letter(i, dictionary_creator::Language::German) << "] " << i << std::endl;
	}
}

void simple_tests::test_one_line_parser()
{
	std::cout << "\nTesting parsing of one line\n";

	dictionary_creator::DictionaryManager english(dictionary_creator::Language::English);
	dictionary_creator::utf8_string estring{ "This is an example of an english string of plain text. The word text here is encountered twice.\n" };
	english.parse_one_line(estring);

	std::cout << "\nSource line:\n" << estring << "\nUndefined words parsed out of that line:\n";
	for (auto i: english.get_undefined())
	{
		std::cout << '\t' << i->get_word() << '\n';
	}
	std::cout << std::endl;

	dictionary_creator::DictionaryManager russian(dictionary_creator::Language::Russian);
	dictionary_creator::utf8_string rstring{ u8"Это пример русской строки, содержащей только текст. Слово текст встречается дважды. Пока всё.\r\n" };
	russian.parse_one_line(rstring);

	std::cout << "\nSource line:\n" << rstring << "\nUndefined words parsed out of that line:\n";
	for (auto i: russian.get_undefined())
	{
		std::cout << '\t' << i->get_word() << '\n';
	}
	std::cout << std::endl;
}

void simple_tests::test_serialization()
{
	std::cout << "\nTesting serialization of dictionary\n";

	auto generalized = [] (dictionary_creator::Language language,
				dictionary_creator::utf8_string langname,
				std::vector<dictionary_creator::utf8_string> words_requested)
	{
		dictionary_creator::DictionaryManager dictionary_object(language);
		dictionary_object.rename(dictionary_creator::utf8_string{ "Testing serialization for " } + langname + " dictionary");

		for (auto i: words_requested)
		{
			dictionary_object.lookup_or_add_word(i);
		}

		auto words = dictionary_object.get_undefined();
		for (auto i: words)
		{
			std::cout << "\tdefining \'" << i->get_word() << "\'...";
			dictionary_object.define(i);
			std::cout << (i->is_defined() ? " done" : " couldn't be done") << std::endl;
		}
		if (auto p = dictionary_object.lookup_or_add_word(words_requested.back()); p)
		{
			p->increment_counter();
		}

		dictionary_object.save_dictionary();
		std::cout << "Saved \"" << dictionary_object.get_name() << "\" dictionary\n";

		auto available = dictionary_creator::available_dictionaries();
		std::cout << "Choose a dictionary to load:\n";
		size_t choice = 0;
		for (size_t i = 0; i != available.size(); ++i)
		{
			std::cout << '\t' << i << ") " << available[i].human_readable << '\n';
		}
		std::cin >> choice;

		std::cout << "Loading \'" << available[choice].human_readable << "\' dictionary\n";

		auto loaded = dictionary_creator::load_dictionary(available[choice].full);

		std::cout << "Loaded object type is " << typeid(loaded).name() << " and has name " << loaded.get_name() << std::endl;
		auto loaded_undefined = loaded.get_undefined();
		std::cout << "It has " << loaded_undefined.size() << " undefined words\n";

		auto top_frequent = loaded.get_subset(dictionary_creator::ComparisonType::MostFrequent, 8);
		std::cout << top_frequent.size() << " most frequent words:\n";
		for (auto i: top_frequent)
		{
			std::cout << '\t' << i->get_word() << "\t[" << i->get_counter() << "]\n";
			for (auto j: i->get_definitions())
			{
				std::cout << '\t' << j.first << ":\n";
				size_t k = 0;
				for (auto l: j.second)
				{
					std::cout << "\t\t" << ++k << ") " << l << "\n";
				}
			}
		}
	};

	generalized(dictionary_creator::Language::English, "english", { "crazy", "crab" });
	generalized(dictionary_creator::Language::Russian, "russian", { u8"волшебный", u8"дирижабль", u8"подземелья" });
	std::cout << "\nThe end." << std::endl;
}

int main(int argc, char **argv)
{
	simple_tests::tests("Dictionary creator and its' features");

	return 0;
}
