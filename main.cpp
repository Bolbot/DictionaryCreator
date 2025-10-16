#include <typeinfo>
#include <chrono>
#include <locale>
#include <cstring>

#include "dictionary_manager.h"
#include "fs_manager.h"
#include "config.h"

// GoogleAPI limits TODO:	develop a strategy for getting around the Google limit
// 				problem: starting from some substantial amount of requests google gets exausted
// 				as a result HTTP 403 response is returned regardless of any further requests
// 				solution may involve having a localy stored json objects for each url
// 				but how to store them all in a way alowing to access them quick? that is the question
//
// German TODO:			Design and implement the approach to parsing german words
// 				current issue: capitalized nouns are treated as proper nouns
//
// Test fix TODO:		Find a way around bugged boost test output stream in win x86
// 				or
// 				rewrite dependent tests with no relying on that one
//
// More tests TODO:		Test dictionaries with more than one word for each letter
// 				Test поезд поёт поэт
// 				Test écran enchanteé et commencer commençon
// 				Test some german umlaut words
// 				Test files (ru/ru, en/en, en/ru, ru/en, jap/ru, ru/fr) as another boost test
//
// Completeness TO-DO:	provide some missing features
// 			list them below:
// 				special exporting options for export_top member function (encounters if sorted by frequency)
// 			CANCELLED since that is unnecessary
//
// Premature optimization TODO: 	consider using custom allocation for storing dictionary
// 					deduce required space upon dictionary loading
// 					how to resize when more space is needed during the work?
// 					what if there is not enough allocated space?
//
// Most distant TODO: implement task-based or thread-pool-based concurrent beforehand web-api processing of dictionary words
// 			find a bottleneck in processing
// 			see how it can be dealt with except for multithreading
// 			develop a divide et impera processing strategy when everything else is applied
//
// UI/UX TODO:	Design GUI considering the most plausible use scenario, make it handy for one or two simple things
// 		Make it elsewhere and try to use interim versions while reasoning about the design


template <typename Lambda, typename TimeUnit = std::chrono::milliseconds>
auto execution_time(Lambda wrapped_task)
{
	auto before = std::chrono::steady_clock::now();
	wrapped_task();
	auto after = std::chrono::steady_clock::now();

	return std::chrono::duration_cast<TimeUnit>(after - before);
}

void manage_dictionary()
{
	auto input_file_path = get_input_file_path();
		
	dictionary_creator::DictionaryManager dm(dictionary_creator::Language::English);

	dm.add_input_file(input_file_path);
	dm.lookup_or_add_word(u8"Antidisestablishmentarianism");

	try
	{
		dm.parse_all_pending();
	}
	catch (std::exception &e)
	{
		std::cerr << "THROWN: " << e.what() << '\n';
	}

	auto longest_20 = dm.get_subset(dictionary_creator::ComparisonType::Longest, 20);
	std::cout << "Longest " << longest_20.size() << " words are:\n";
	for (const auto &i : longest_20)
	{
		std::cout << '\t' << i->get_word() << '\n';
	}
	std::cout << "Those are going to be defined.\n";
	dm.define(longest_20);

	auto output_file_path = get_output_file_path();

	std::ofstream output_stream(output_file_path);

	dm.set_output(output_stream);
	dm.export_dictionary();

	std::cout << "Exporting done" << std::endl;
}

int main (int argc, char **argv)
{
	std::cout << PROJECT_NAME << ' ' << PROJECT_VERSION << '\n';
	if (UTF8_aware_console == false)
	{
		std::cout << "Warning: Windows console output isn't UTF-8 aware!" << std::endl;
	}

	if (argc == 2 && strcmp(argv[1], "TESTRUN") == 0)
	{
		std::cout << "Test run. C++ version is " << __cplusplus << "\n";
		return 0;
	}

	manage_dictionary();

	return 0;
}
