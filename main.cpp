#include <typeinfo>
#include <chrono>
#include <locale>

#include "dictionary_manager.h"
#include "filesystem_features.h"
#include "config.h"

// GoogleAPI limits TODO:	develop a strategy for getting around the Google limit
// 				problem: starting from some substantial amount of requests google gets exausted
// 				as a result HTTP 403 response is returned regardless of any further requests
// 				solution may involve having a localy stored json objects for each url
// 				but how to store them all in a way alowing to access them quick? that is the question
//
// Refinement TO-DO:	make load_dictionary return the DictionaryManager objects with names corresponding to requested ones DONE
// 			test it and make sure it works DONE
//
// Connections hotfix TO-DO:	look into the unicode containing URL issue, find a solution to request UTF letters				DONE
// 				implement it either in dictionary_definer.cpp define_word() or in connections.cpp in constructor		DONE
// 				if it takes adding third party libraries, link them in cmake and run up some separate simple_tests for them	NOPE
// 				update test_connections to test against words 'naïve', 'scheiße', and 'дирижабль'				DONE
//
// Console interface TO-DO:	separate windows-related and unix-related checks
// 				implement a consistent working and uniform scenario for opening and saving files via main
// 					either
// 						use std::filesystem to deal with platform-related issues	// total NO - useless garbage
// 					or
// 						provide a set of platform-aware headers dealing with filenames and opening files	DONE
//					revise the interfaces providing an overload for std::wstring file names and use those with Windows DONE
// 				test it using different languages in file names and expected contents (ru/ru, en/en, en/ru, ru/en, jap/ru, ru/fr) DONE
//
// Completeness TODO:	provide some missing features
// 			list them below:
// 				special exporting options for export_top member function (encounters if sorted by frequency)
//
// Premature optimization TODO: 	consider using custom allocation for storing dictionary
// 					deduce required space upon dictionary loading
// 					how to resize when more space is needed during the work?
// 					what if there is not enough allocated space?
//
// Test convenience TO-DO:	revise testing approach DONE
// 				redesign inner structure of the project to build a dictionary_creator lib apart from tests and executables DONE
// 				engage some unit test framework and provide regression testing in separate subdirectories and executables  DONE
// 				mark existing huge_tests and simple_test as obsolete and move there	DONE
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

auto get_input_file_path()
{
	std::cout << "Specify input file:\n";

	while (true)
	{
		auto file_path = read_user_input_line();

		remove_enclosing_quotes(file_path);

		if (valid_file_path(file_path) == false)
		{
			std::cout << "Invalid file path. Try another.\n";
			continue;
		}

		if (file_already_exists(file_path) == false)
		{
			std::cout << "File does not exist. Try another.\n";
			continue;
		}

		if (regular_file(file_path) == false)
		{
			std::cout << "Not a file. Try another.\n";
			continue;
		}
		
		return file_path;
	}
}

auto get_output_file_path()
{
	std::cout << "Specify output file (.txt if no other extension specified):\n";

	while (true)
	{
		auto file_path = read_user_input_line();

		remove_enclosing_quotes(file_path);

		if (valid_file_path(file_path) == false)
		{
			std::cout << "Invalid file path. Try another.\n";
			continue;
		}

		fix_missing_extension(file_path);

		if (file_already_exists(file_path))
		{
			std::cout << "File already exists. Lose it's previous contents and write there anyway? ([Y]es / [n]o)\n> ";

			char choice = char{};
			std::cin >> choice;
			std::cin.ignore(std::cin.rdbuf()->in_avail(), '\n');

			if (choice != 'Y')
			{
				std::cout << "Specify another file.\n";
				continue;
			}
		}

		if (std::ofstream(file_path).good() && regular_file(file_path) == false)
		{
			std::cout << "Not a regular file. Try another.\n";
			continue;
		}

		return file_path;
	}
}

void manage_dictionary()
{
	auto input_file_path = get_input_file_path();
		
	dictionary_creator::DictionaryManager dm(dictionary_creator::Language::Russian);

	dm.add_input_file(input_file_path);
	dm.lookup_or_add_word(u8"космос");

	try
	{
		dm.parse_all_pending();
	}
	catch (std::exception &e)
	{
		std::cerr << "THROWN: " << e.what() << '\n';
	}

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
	dm.define(longest_7);

	auto output_file_path = get_output_file_path();

	std::ofstream output_stream(output_file_path);

	dm.set_output(output_stream);
	dm.export_dictionary();

	std::cout << "Exporting done" << std::endl;
}


int main (int argc, char **argv)
{
	std::cout << PROJECT_NAME_GENERAL << ' ' << PROJECT_VERSION_MAJOR << '.'
		<< PROJECT_VERSION_MINOR << '.'	<< PROJECT_VERSION_PATCH << '\n';
#ifdef _WIN32
	if (!UTF8_awareness_for_windows_console)
	{
		std::cout << "Warning: Windows console output isn't UTF-8 aware!" << std::endl;
	}
#endif
	if (argc == 2 && strcmp(argv[1], "TESTRUN") == 0)
	{
		std::cout << "Test run. C++ version is " << __cplusplus << " (if it's UNIX)\n";
		return 0;
	}

	manage_dictionary();

	return 0;
}
