#include <clocale>
#include <typeinfo>

#include "utils.h"
#include "dictionary_creator.h"

// DO NOT!!! Don't test web lookup on huge files because this leads to 403 error and abortion of application
// TO-DO:	check HTTP status before procceeding with any presumable response content
//		provide error messages in case of unsuccessfull response status upon lookup attempts	DONE
//
// Reasonable TODO: store definitions alongside the words, also provide the option for disabling it
// Provide an Entry class for words with definitions and store like { lowcase string type, map<part of speech, set of definitions> }
// Implement deferred lookup not to abuse the web api
// Revise the DictionaryCreator class to store words as Entry
//
// Distant TODO: implement encounter counter and possibility to sort by it
//
// Most distant TODO: implement task-based or thread-pool-based concurrent beforehand web-api processing of non-capitalized words

void tests()
{
	output << "Tests\n";

	bool test_json = false;
	bool test_connections = true;
	bool test_regex = false;
	bool test_lowercase_words = false;
	bool test_Entry_class = false;

	if (test_json)
	{
		std::wstring must_be_json{ LR",,,([{"word":"hello","phonetics":[{"text":" / həˈloʊ / ","audio":"https://lex-audio.useremarkable.com/mp3/hello_us_1_rr.mp3"},{"text":"/hɛˈloʊ/","audio":"https://lex-audio.useremarkable.com/mp3/hello_us_2_rr.mp3"}],"meanings":[{"partOfSpeech":"exclamation","definitions":[{"definition":"Used as a greeting or to begin a phone conversation.","example":"hello there, Katie!"}]},{"partOfSpeech":"noun","definitions":[{"definition":"An utterance of “hello”; a greeting.","example":"she was getting polite nods and hellos from people","synonyms":["greeting","welcome","salutation","saluting","hailing","address","hello","hallo"]}]},{"partOfSpeech":"intransitive verb","definitions":[{"definition":"Say or shout “hello”; greet someone.","example":"I pressed the phone button and helloed"}]}]}]),,," };

		try
		{
			auto some_json = parse_json(must_be_json);

			auto definition = dict::single_definition(some_json);
			output << "One definition is: " << definition << std::endl;

			auto set = dict::set_of_definitions(some_json);
			output << set.size() << " definitions:\n";
			for (const auto &x : set)
			{
				output << '\t' << x << std::endl;
			}
			output << std::endl;

			auto pos = dict::set_of_part_of_speech_definitions(some_json);
			output << set.size() << " definitions with parts of speech:\n";

			for (const auto &x : pos)
			{
				output << "as " << x.first << ":\n";
				for (const auto &d : x.second)
				{
					output << '\t' << d << std::endl;
				}
			}
			output << std::endl;

			auto irrelevant = parse_json(connections::get("https://yahoo.com"));
			auto irrdef = dict::single_definition(irrelevant);
			output << "\n\nIRRELEVANT page. Single definition: " << irrdef << std::endl;
			auto irrpos = dict::set_of_part_of_speech_definitions(irrelevant);
			output << "Set of part of speech for irrelevant has size: " << irrpos.size() << std::endl;
		}
		catch (std::exception &e)
		{
			std::cerr << "It throws: " << e.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "It throws something that can't be caught\n";
		}
	}

	if (test_connections)
	{
		output << "testing connections\n";

		auto http = connections::get("http://www.google.com");
		output << "http: " << (http.size() > 100 ? "successfull" : http.data()) << std::endl;
		
		auto https = connections::get("https://api.dictionaryapi.dev/api/v2/entries/en/hello");
		output << "https: " << (https.size() > 100 ? "successfull" : https.data()) << std::endl;

		auto nonexistent = connections::get("https://nonexis.tent.pa.ge.com");
		output << "nonexistent page: " << nonexistent << std::endl;

		auto notfound = connections::get("https://google.com/pagecan/notbe.found");
		output << "not found: " << notfound << std::endl;
	}

	if (test_regex)
	{

		output << "testing regular expressions\n";

		std::string pattern{ R"(Z[a-z]{4,6}\b)" };
		pcre_parser::RegexParser parser(pattern.c_str());
		std::string line{ "My name is Zorro\n- What? Was it Zero or Zealot or Zombie?\n- No, Zorro!" };
		std::cout << "Given source:\n" << line << "\nSearching with pattern \'" << pattern << "\'\n";

		std::cout << "Single match result: \'" << parser.single_match(line) << "\'\n";

		std::cout << "Every match: ";
		for (const auto &i: parser.all_matches(line))
		{
			std::cout << i << ' ';
		}
		std::cout << std::endl;
	}

	if (test_lowercase_words)
	{
		WordType word = "The Great Initital StrinG";
		std::cout << "Type any strings or ! to terminate\n";
		while (word[0] != '!')
		{
			getline(input, word);
			output << word << std::endl;
		}
	}

	if (test_Entry_class)
	{
		WordType word{ "FRAME" };
		dict::Entry entry(word);
		output << entry.get_word() << std::endl;
		output << "Size of entry with no definitions is " << sizeof(entry) << std::endl;
		entry.find_definitions();
		output << "Size of entry with definitions is " << sizeof(entry) << std::endl;
		for (const auto &[pos,definitions]: entry.get_definitions())
		{
			output << "As " << pos << std::endl;
			for (const auto &d: definitions)
			{
				output << "\t\t" << d << std::endl;
			}
			if (definitions.size() > 1)
			{
				output << std::endl;
			}
		}
	}
}

#ifdef __unix__
int main
#else
int wmain
#endif
(int argc, fsCharType **argv)
{
	setlocale(LC_ALL, "Russian_Russia.1251");
	output << "DictionaryCreator 0.7\n";

	bool make_tests_not_work = false;

	if (make_tests_not_work)
	{
		tests();
		return 0;
	}

	FSStringType current_directory{ argv[0] };
	current_directory.erase(current_directory.find_last_of(slash_or_backslash), StringType::npos);
	
	dict::DictionaryCreator dc(current_directory);
	dc.request_the_extensions();

	auto start = std::chrono::steady_clock::now();

	dc.parse_all_files();

	auto parsed = std::chrono::steady_clock::now();

	dc.export_dictionary("output_DICTIONARY.txt", dict::export_data::AllDefinitionsPerPartOfSpeech);

	auto exported = std::chrono::steady_clock::now();

	FileOutputStream benchmark("benchmark_results.txt");

	benchmark << "Parsed in " << std::chrono::duration_cast<std::chrono::milliseconds>(parsed - start).count() << " ms\n"
		<< "Exported in " << std::chrono::duration_cast<std::chrono::milliseconds>(exported - parsed).count() << " ms" << std::endl;

	dc.export_proper_nouns("output_ProperNouns_by_DICTIONARY.txt");

	return 0;
}
