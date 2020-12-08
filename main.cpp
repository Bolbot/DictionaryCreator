#include <clocale>
#include <typeinfo>

// definitions.h must be included after portability_utils.h but before any other project headers
#include "utils.h"
#include "dictionary_creator.h"

// Short TODO: fix not recognizing the last word of text file as a name when it is
//
// Further TO-DO: implement the part of speech as
// (noun):	Definition of this word as a noun
// (adjective):	Definition of this word as an adjective
// MAYBE provide another type of output:
// As noun:	Definition of this word as noun. It's the only one here, so in the same string.
// As verb:
// 		1) First definition as verb
// 		2) Second definition as verb etc.
// As adjective:	Definition as adjective, if there's a lot of definitions, this should be more convenient
//
// Portability TODO: provide portable alternative to cpr for Windows since issues
// Issues: cpr.dll cannot be found and causing built program to fail upon the execution start
//
// Distant TODO: implement encounter counter and possibility to sort by it
//
// Most distant TODO: implement task-based or thread-pool-based concurrent beforehand web-api processing of non-capitalized words

void tests()
{
	output << "Tests\n";
	std::wstring must_be_json{ LR",,,([{"word":"hello","phonetics":[{"text":" / həˈloʊ / ","audio":"https://lex-audio.useremarkable.com/mp3/hello_us_1_rr.mp3"},{"text":"/hɛˈloʊ/","audio":"https://lex-audio.useremarkable.com/mp3/hello_us_2_rr.mp3"}],"meanings":[{"partOfSpeech":"exclamation","definitions":[{"definition":"Used as a greeting or to begin a phone conversation.","example":"hello there, Katie!"}]},{"partOfSpeech":"noun","definitions":[{"definition":"An utterance of “hello”; a greeting.","example":"she was getting polite nods and hellos from people","synonyms":["greeting","welcome","salutation","saluting","hailing","address","hello","hallo"]}]},{"partOfSpeech":"intransitive verb","definitions":[{"definition":"Say or shout “hello”; greet someone.","example":"I pressed the phone button and helloed"}]}]}]),,," };

	try
	{
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

#ifdef __unix__
int main
#else
int wmain
#endif
(int argc, fsCharType **argv)
{
	setlocale(LC_ALL, "Russian_Russia.1251");
	output << "DictionaryCreator 0.5\n";

	FSStringType current_directory{ argv[0] };
	current_directory.erase(current_directory.find_last_of(slash_or_backslash), StringType::npos);
	
	
	dict::DictionaryCreator dc(current_directory);
	dc.request_the_extensions();
	dc.parse_all_files();
	dc.export_dictionary("output_DICTIONARY.txt", dict::export_data::AllDefinitionsPerPartOfSpeech);
	
	//tests();

	return 0;
}
