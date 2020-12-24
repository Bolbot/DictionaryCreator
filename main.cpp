#include <clocale>
#include <typeinfo>

#include "utils.h"
#include "dictionary_creator.h"


// Short TO-DO: fix not recognizing the last word of text file as a name when it is DONE
// Also: fix not recognizing multiword names as names, that is:
// 	must be:	Antonio Margheritti =====> name Antonio; name Margheritti
//	currently:	Antonio Margheritti =====> name Antonio; word margheritti
//	TO-DO: fix this (probably better to fix this first, cause then last name ignoration is likely to be fixed also DONE
// Short fix: do NOT take the first word int text file as a proper noun even if it's capitalized, TO-DO - this fix - DONE
// Necessary TO-DO: fix the regular expression for the names DONE
//
// Further TO-DO: implement the part of speech as
// (noun):	Definition of this word as a noun
// (adjective):	Definition of this word as an adjective
// TO-DO MAYBE provide another type of output:
// As noun:	Definition of this word as noun. It's the only one here, so in the same string.
// As verb:
// 		1) First definition as verb
// 		2) Second definition as verb etc.
// As adjective:	Definition as adjective, if there's a lot of definitions, this should be more convenient
// DONE
//
// Portability TO-DO: provide portable alternative to cpr for Windows since issues DONE
// Issues: cpr.dll cannot be found and causing built program to fail upon the execution start
//
// Short TODO: fix regular words capitalization - maybe use string traits from optional_utils for storing words
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

	if (test_json)
	{
		std::wstring must_be_json{ LR",,,([{"word":"hello","phonetics":[{"text":" / həˈloʊ / ","audio":"https://lex-audio.useremarkable.com/mp3/hello_us_1_rr.mp3"},{"text":"/hɛˈloʊ/","audio":"https://lex-audio.useremarkable.com/mp3/hello_us_2_rr.mp3"}],"meanings":[{"partOfSpeech":"exclamation","definitions":[{"definition":"Used as a greeting or to begin a phone conversation.","example":"hello there, Katie!"}]},{"partOfSpeech":"noun","definitions":[{"definition":"An utterance of “hello”; a greeting.","example":"she was getting polite nods and hellos from people","synonyms":["greeting","welcome","salutation","saluting","hailing","address","hello","hallo"]}]},{"partOfSpeech":"intransitive verb","definitions":[{"definition":"Say or shout “hello”; greet someone.","example":"I pressed the phone button and helloed"}]}]}]),,," };

		try
		{
#ifdef __NLOHMANN_JSON_IS_AVALIABLE__
			auto some_json = json::parse(must_be_json);

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
#else
			output << "nlohmann json feature is unavaliable\n";
#endif
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
	dc.parse_all_files();
	dc.export_dictionary("output_DICTIONARY.txt", dict::export_data::AllDefinitionsPerPartOfSpeech);
	dc.export_proper_nouns("output_ProperNouns_by_DICTIONARY.txt");

	return 0;
}
