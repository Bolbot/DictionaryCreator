#include <typeinfo>
#include <chrono>
#include <iostream>
#include <locale>

//#include "config.h"	// TODO: make this a conjunction of platform name and headers availability
/*#if WINDOWS_H_IS_AVAILABLE && IO_H_IS_AVAILABLE*/
#if defined _WIN32 && __has_include(<Windows.h>) 
#include <io.h>
#include <Windows.h>
bool UTF8_awareness_for_windows_console = [] () { return SetConsoleOutputCP(CP_UTF8) && SetConsoleCP(CP_UTF8); }();
#endif

#include "simple_tests.h"
#include "dictionary_manager.h"

// DO NOT!!! Don't test web lookup on huge files because this leads to 403 error and abortion of application
// TO-DO:	check HTTP status before procceeding with any presumable response content
//		provide error messages in case of unsuccessfull response status upon lookup attempts	DONE
//
// Crucial fix TODO:	revise the approach of defining, definer_t definer does not belong to Dictionary class,
// 			PROBLEM: it's instantiated for each other object while entries keep dangling references
// 			PROBABLE FIX: aim towards creating a single instance of such a definer somewhere else
// 					so that Dictionary could keep a reference just like Entry does
// 					each time new Dictionary is created that definer is retrieved from somewhere
// 					if the language matches, then it's gonna be the reference to the same object
// 			REQUIREMENT: lifetime of the definer must match the program lifetime so that dangling reference is practically impossible
// 			ARCHITECTURE: consider having a singletone definer, make_definer(Language) function, and array along with language stuff
//
// Windows TODO:	Teach Windows to open files with spaces (\screening suffices) and unicode characters in path
//
//
// Architecture TODO:	ensure inner consistency of classes DONE
// 				complete inner reimplementation debt TODOs DONE
// 				move type aliases to separate dictionary_types.h header DONE
// 			rewrite stored types from variable Entries to shared_ptr<Entry>
// 				reimplement all functions dealing with Entry
// 				make Entry type polymorphic (add virtual destructor)
// 				provide custom sort with one more possible ComparisonType::Custom
// 				create one more custom sort overload accepting template Comp if ::Custom
// 				wrap that custom in another lambda dealing with results of dynamic_cast<> failure
// 			find every place that is creating an Entry and make it template, accepting CustomEntry type with Entry as default
//			check if necessity in const remains after turn to pointer storing, get rid of it if possible
//
// Linking TO-DO:	clarify the structure of the project DONE, see below
// 			rewrite the CMakeLists.txt according to that structure DONE
// 			make sure everything compiles and links DONE
//
// 			STRUCTURE
//
// 			DictionaryEntry -- self-sufficent as .h and .cpp, declares aliases for utf8_string, definitions_t and definer_t
// 			Dictionary and DictionaryExporter -- requires NlohmanJson, connectionss, and DictionaryEntry linked and includes dictionary_language.h
// 			DictionaryCreator -- requires regex_parser and Dictionary linked
// 			DictionaryManager -- user interface, requires Dictionary, and DictionaryCreator linked
// 						also needs to have a complete interface accorting to following
//
// Interface TO-DO:	assure interface integrity and sufficency according to anticipated use scenario:
// 			add file streams of files to be parsed
// 			parse all pending files to dictionary
// 			lookup for separate words or add them
// 			retrieve custom words subset
// 			export previous or all to given file stream
//
// TDD Routine TODO:	write tests for the interface features DONE
//			provide all the interface features developed DONE
//			when all beforementioned works consider interface development DONE
//			write a huge complex test involving all of those features
//			check its work on two languages: english and russian
//
// Convenience TO-DO:	get rid of redundant aliases, that is DictionaryEntryType for Entry DONE
// 			make sure all member function names are specific enough DONE
//
// Portability TO-DO:	get rid of ugly and unnecessary preprocessor directives provided in false attempts to crossplatform
// 			make sure every text occurence is stored in valid UTF-8 std::string
// 			provide an alias for it nontheless, in case of future development beyond <char> typed string
//
// Modularity TO-DO:	revise structure, main unit shall be rather simple Dictionary class with its support classes:
// 				Entry stores processed words along with their definitions and some supplement info, flags etc.
// 				FileParser provides interface accepting file names or UTF-8 aware streams with LanguageInfo
// 				LanguageInfo provides generalized patterns for regular expression parsing
// 			Dictionary may be unaware of language it stores, but LanguageInfo must provide it preventing to join mismatching objects
// 			Previously mentioned should belong to one namespace, although may be contained in different translation units
//
// Unicode TO-DO:	Revise implementations of classes dealing with letter - it must be LetterType -- unnecessary difficulties and obscuration
// 			provide LetterType get_first_letter(WordType); function -- DONE
//
// Further features TO-DO: 	choose and provide features for further use like get_random_word, DONE
// 				consider allowance to derive from Entry to supply custom info and sorter types DONE, verdict: POSSIBLE
// 				sorting by priority, storing custom info like priority, difficulty etc.
//
// Premature optimization TODO: consider using custom allocation for storing dictionary
//
// Options TO-DO: provide a define_some_words function, taking vector of pointers to entries to be defined DONE
//
// Most distant TODO: implement task-based or thread-pool-based concurrent beforehand web-api processing of non-capitalized words
// 			DEBATABLE NECESSITY
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
	std::cout << "Tests\n";

	bool test_json = false;
	bool test_regex = true;
/*
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
	*/
}

void manage_dictionary()
{
	std::cout << "Write full path of file to add it to dictionary\n> " << std::flush;

	while (true)
	{
		std::string path;
		std::cin.imbue(std::locale("en_US.UTF-8"));
		std::getline(std::cin, path);

		std::ifstream file(path);
		if (file.good())
		{
			std::cout << "Valid file, trying to process...\n";
		}
		else
		{
			std::cout << "Can't process this file, seems invalid\n";
			std::cin.clear();
		}
	}

	std::string path;
	std::getline(std::cin, path);

	std::ifstream file(path);
	if (file.good())
	{
		std::cout << "Valid file, trying to process...\n";
	}
	else
	{
		std::cout << "Can't process this file, seems invalid\n";
		return;
	}

	dictionary_creator::DictionaryManager dm(dictionary_creator::Language::English);

	dm.add_input_file(std::move(file));
	dm.lookup_or_add_word(u8"Antidisestablishmentarianism");

	dm.parse_all_pending();

	auto longest_7 = dm.get_subset(dictionary_creator::ComparisonType::Longest, 7);
	if (longest_7.size() == 7)
	{
		std::cout << "Longest 7 words are:\n";
		for (const auto &i : longest_7)
		{
			std::cout << '\t' << i->get_word() << '\n';
		}
		std::cout << "Those are going to be defined.\n";
	}
	//dm.define(longest_7);

	std::cout << "Write the output file name\n> ";
	std::string output_file;
	while (output_file.empty())
	{
		std::getline(std::cin, output_file);
	}

	if (std::ifstream(output_file).good())
	{
		std::cout << "This is an existing file! Are you sure you want to discard it's content and write there? (Y/n)\n";
		if (std::cin.get() != 'Y')
		{
			std::cout << "Then restart the program, sorry." << std::endl;
			return;
		}
	}

	std::ofstream output(output_file, std::ios_base::out | std::ios_base::trunc);
	dm.set_output(output);
	dm.export_dictionary();

	std::cout << "Exporting done. See the contents of " << output_file << ".\nTHE END" << std::endl;
}

int main (int argc, char **argv)
{
	auto program_title = u8"DictionaryCreator 0.8.2";
	std::cout << program_title << std::endl;
#ifdef _WIN32
	if (!UTF8_awareness_for_windows_console)
	{
		std::cout << "Warning: Windows console output isn't UTF-8 aware!" << std::endl;
	}
#endif

	bool make_tests_not_work = false;

	if (argc > 1 && !strncmp(argv[1], "TEST", 4))
	{
		make_tests_not_work = true;
	}

	if (make_tests_not_work)
	{
		simple_tests::tests(program_title);
		return 0;
	}
	else
	{
		manage_dictionary();
	}

	return 0;
}
