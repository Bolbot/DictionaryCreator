#include <clocale>
#include <typeinfo>
#include <chrono>

#include "utils.h"
#include "console_file_system.h"
#include "dictionary_creator.h"

// DO NOT!!! Don't test web lookup on huge files because this leads to 403 error and abortion of application
// TO-DO:	check HTTP status before procceeding with any presumable response content
//		provide error messages in case of unsuccessfull response status upon lookup attempts	DONE
//
// Convenience TODO:	get rid of redundant aliases, that is DictionaryEntryType for Entry DONE
// 			make sure all member function names are specific enough
//
// Portability TODO:	get rid of ugly and unnecessary preprocessor directives provided in false attempts to crossplatform
// 			make sure every text occurence is stored in valid UTF-8 std::string
// 			provide an alias for it nontheless, in case of future development beyond <char> typed string
//
// Modularity TODO:	revise structure, main unit shall be rather simple Dictionary class with its support classes:
// 				Entry stores processed words along with their definitions and some supplement info, flags etc.
// 				FileParser provides interface accepting file names or UTF-8 aware streams with LanguageInfo
// 				LanguageInfo provides generalized patterns for regular expression parsing
// 			Dictionary may be unaware of language it stores, but LanguageInfo must provide it preventing to join mismatching objects
// 			Previously mentioned should belong to one namespace, although may be contained in different translation units
//
// Unicode TODO:	Revise implementations of classes dealing with letter - it must be LetterType,
// 			provide LetterType get_first_letter(WordType); function
//
// Further features TODO: choose and provide features for further use like get_random_word,
// 				sorting by priority, storing custom info like priority, difficulty etc.
//
// Premature optimization TODO: consider using custom allocation for storing dictionary
//
// Options TODO: provide a define_some_words function, taking vector of pointers to entries to be defined
//
// Most distant TODO: implement task-based or thread-pool-based concurrent beforehand web-api processing of non-capitalized words
//
// Serious TODO: provide a GUI
// First of all TODO: formulate GUI requirements and features
//
// TODO:	refine GUI requirements
// 		decide where to place necessary language setting and when to ask user for it if even ask
// 		simplify GUI from user perspective when it's refined, simplify as much as ever possible
// 
//	GUI requirements
// GUI contains list of text files to be analyzed,
// 		option to add new files via windows explorer (or drag-n-drop to the list section also)
// 		settings section that has the default settings (which one?) and possibility to open advanced settings
// 			advanced settings on request that provide definition style, exporting separate files with words sorted etc.
// 		filled by default field of output dictionary file name, user can edit it or leave as it is
// 		if analyzed files belong to different location, gui request user for destination for results
//
// 		Refined version
// 		Menu bar:
// 			File:
// 				Add --- add file to be parsed
// 				Clear all --- remove all currently pending files
// 				-----------------
// 				Export dictionary --- export parsing results with default options
// 				Export top --- export partially, custom sorted, ask to specify how and how many
// 				Export proper nouns --- export proper nouns
// 				-----------------
// 				Quit --- close the application, ask for confirm if any unprocessed files left
// 			Process:
// 				Process all --- parse all pending files
// 				----------------
// 				Display top --- display topmost custom sorted words in separate window
// 				Display proper nouns --- display all the proper nouns in separate window
//			Settings:
//				Definitions --- choose between different definitions options from none to all
//				File info --- choose between fullpath, only names, or no information about the source files
//
//		List view:
//			Table or list of pending files to be processed
//			(perhaps) already processed files as greyed out lines
//
//		
//				

template <typename Lambda, typename TimeUnit = std::chrono::milliseconds>
auto execution_time(Lambda wrapped_task)
{
	auto before = std::chrono::steady_clock::now();
	wrapped_task();
	auto after = std::chrono::steady_clock::now();

	return std::chrono::duration_cast<TimeUnit>(after - before);
}

void tests()
{
	output << "Tests\n";

	bool test_json = false;
	bool test_connections = false;
	bool test_regex = true;
	bool test_lowercase_words = true;
	bool test_Entry_class = true;

	if (test_json)
	{
		output << "\nTesting json\n";

		std::wstring must_be_json{ LR",,,([{"word":"hello","phonetics":[{"text":" / həˈloʊ / ","audio":"https://lex-audio.useremarkable.com/mp3/hello_us_1_rr.mp3"},{"text":"/hɛˈloʊ/","audio":"https://lex-audio.useremarkable.com/mp3/hello_us_2_rr.mp3"}],"meanings":[{"partOfSpeech":"exclamation","definitions":[{"definition":"Used as a greeting or to begin a phone conversation.","example":"hello there, Katie!"}]},{"partOfSpeech":"noun","definitions":[{"definition":"An utterance of “hello”; a greeting.","example":"she was getting polite nods and hellos from people","synonyms":["greeting","welcome","salutation","saluting","hailing","address","hello","hallo"]}]},{"partOfSpeech":"intransitive verb","definitions":[{"definition":"Say or shout “hello”; greet someone.","example":"I pressed the phone button and helloed"}]}]}]),,," };
		std::wstring russian_json{ LR",,,([{"word":"ГУЛЯ́ТЬ","phonetics":[{}],"meanings":[{"partOfSpeech":"undefined","definitions":[{"definition":"Совершать прогулку.","example":"Г. с детьми","synonyms":[],"antonyms":[]},{"definition":"Быть свободным от работы, иметь выходной день разг..","example":"Гуляли два дня","synonyms":[],"antonyms":[]},{"definition":"Кутить, веселиться прост..","example":"Г. на свадьбе","synonyms":[],"antonyms":[]},{"definition":"Быть в близких, любовных отношениях прост..","synonyms":[],"antonyms":[]}]}]}]),,," };

		try
		{
			auto some_json = parse_json(russian_json);
			
			auto definition = dict::single_definition(some_json);
			output << "One definition is: " << definition << std::endl;

			auto set = dict::set_of_definitions(some_json);
			output << "Set of " << set.size() << " definitions:\n";
			for (const auto &x : set)
			{
				output << '\t' << x << std::endl;
			}
			output << std::endl;

			output << "Map of all definitions:\n";
			auto map = dict::part_of_speech_definitions_map(some_json);
			for (auto[pos, defs] : map)
			{
				output << pos << "\n";
				for (auto d : defs)
				{
					output << "\t" << d << std::endl;
				}
			}
			output << std::endl;

			auto irrelevant = parse_json(connections::get("https://yahoo.com"));
			auto irrdef = dict::single_definition(irrelevant);
			output << "\n\nIRRELEVANT page. Single definition: " << irrdef << std::endl;
			auto irrpos = dict::part_of_speech_definitions_map(irrelevant);
			output << "Map of part of speech for irrelevant has size: " << irrpos.size() << std::endl;
		}
		catch (std::exception &e)
		{
			error_output << "It throws: " << e.what() << std::endl;
		}
		catch (...)
		{
			error_output << "It throws something that can't be caught\n";
		}
	}

	if (test_connections)
	{
		output << "\n\nTesting connections\n";

		auto http = connections::get("http://www.google.com");
		output << "http: " << (http.size() > 100 ? "successfull" : http.data()) << std::endl;
		
		auto https = connections::get("https://api.dictionaryapi.dev/api/v2/entries/en/hello");
		output << "https: " << (https.size() > 100 ? "successfull" : https.data()) << std::endl;

		auto nonexistent = connections::get("https://nonexis.tent.pa.ge.com");
#if __STRINGTYPE_IS_WSTRING__
		output << "nonexistent page: " << converter_to_wstring.from_bytes(nonexistent) << std::endl;
#else
		output << "nonexistent page: " << nonexistent << std::endl;
#endif

		auto notfound = connections::get("https://google.com/pagecan/notbe.found");
#if __STRINGTYPE_IS_WSTRING__
		output << "not found: " << converter_to_wstring.from_bytes(notfound) << std::endl;
#else
		output << "not found: " << notfound << std::endl;
#endif
	}

	if (test_regex)
	{
		output << "\n\nTesting regular expressions\n";

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
		output << "\n\nTesting lowercase words\n";

#if __STRINGTYPE_IS_WSTRING__
		WordType<wchar_t> word = L"The Great Initital StrinG";
#else
		WordType<char> word = "The Great Initital StrinG";
#endif
		output << word << std::endl;

		output << "Type any strings or ! to terminate\n";
		while (word[0] != '!')
		{
			getline(input, word);
			output << word << std::endl;
		}
	}

	if (test_Entry_class)
	{
		output << "\n\nTesting Entry class\n";

#if __STRINGTYPE_IS_WSTRING__
		WordType<wchar_t> word{ L"FRAME" };
#else
		WordType<char> word{ "FRAME" };
#endif
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
(int argc, fs::fsCharType **argv)
{
	setlocale(LC_ALL, "Russian_Russia.1251");
#if __STRINGTYPE_IS_WSTRING__
	auto program_title = L"DictionaryCreator 0.8";
#else
	auto program_title = "DictionaryCreator 0.8";
#endif
	output << program_title << std::endl;

	bool make_tests_not_work = false;

	if (make_tests_not_work)
	{
		tests();
		return 0;
	}

	fs::FSStringType current_directory{ argv[0] };
	current_directory.erase(current_directory.find_last_of(fs::slash_or_backslash), StringType::npos);
	
	dict::DictionaryCreator dc;
	/*

	fs::request_the_extensions(current_directory);

	auto start = std::chrono::steady_clock::now();

	auto parsed_time = execution_time([&dc]
			{
				auto all_file_streams = fs::provide_all_files_streams();
				for (auto &stream: all_file_streams)
				{
					dc.parse_to_dictionary(std::move(stream));
				}
			});

	auto parsed = std::chrono::steady_clock::now();

//	dc.define_all_words();

	auto defined = std::chrono::steady_clock::now();

#ifdef __THIS_IS_UNIX__
	auto main_dictionary_output = "output_DICTIONARY.txt";
	auto top_10 = "Top 10";
	auto proper_nouns_output = "output_ProperNouns_by_DICTIONARY.txt";
#elif defined __THIS_IS_WINDOWS__
	auto main_dictionary_output = L"output_DICTIONARY.txt";
	auto top_10 = L"Top 10";
	auto proper_nouns_output = L"output_ProperNouns_by_DICTIONARY.txt";
#endif

	dc.export_dictionary(fs::output_to_file(main_dictionary_output), dict::export_data::AllDefinitionsPerPartOfSpeech);

	auto exported = std::chrono::steady_clock::now();
	
	auto top_frequent_time = execution_time([&dc, &top_10]
		{
			auto comparation = dict::DictionaryCreator::ComparationType::MostFrequent;
			dc.export_top(comparation, fs::output_to_file(fs::generate_sorted_file_name(top_10, static_cast<size_t>(comparation))));

			comparation = dict::DictionaryCreator::ComparationType::LeastFrequent;
			dc.export_top(comparation, fs::output_to_file(fs::generate_sorted_file_name(top_10, static_cast<size_t>(comparation))));
		});

	FileOutputStream benchmark("benchmark_results.txt");

	benchmark << "Parsed in " << std::chrono::duration_cast<std::chrono::milliseconds>(parsed - start).count() << " ms\n"
		<< "Defined in " << std::chrono::duration_cast<std::chrono::milliseconds>(defined - parsed).count() << " ms\n"
		<< "Exported in " << std::chrono::duration_cast<std::chrono::milliseconds>(exported - defined).count() << " ms" << std::endl;
	benchmark << "\nMost and least frequent words were exported in " << top_frequent_time.count() << " ms\n"
	       << "Parsed in " << parsed_time.count() << "ms" << std::endl;

	dc.export_proper_nouns(fs::output_to_file(proper_nouns_output));
	*/

	return 0;
}
