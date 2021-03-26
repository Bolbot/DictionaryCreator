#include <typeinfo>
#include <chrono>
#include <iostream>
#include <locale>

#include "config.h"
#if WINDOWS_H_IS_AVAILABLE && IO_H_IS_AVAILABLE
#include <io.h>
#include <Windows.h>

bool UTF8_awareness_for_windows_console = [] () { return SetConsoleOutputCP(CP_UTF8) && SetConsoleCP(CP_UTF8); }();

auto crossplatform_getline = [] (std::istream &input, std::string &dest) -> std::istream &
{
	constexpr size_t WBUFFER_SIZE = 8192;
	constexpr size_t CBUFFER_SIZE = WBUFFER_SIZE * 3 + 1;

	wchar_t wbuffer[WBUFFER_SIZE] { 0 };
	char charbuffer[CBUFFER_SIZE] { 0 };

	HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);
	if (input_handle == nullptr || input_handle == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error("Failed to acquire input handle in MS Windows");
	}

	DWORD read = 0;

	if (ReadConsole(input_handle, wbuffer, WBUFFER_SIZE, &read, nullptr) == 0)
	{
		throw std::runtime_error("Failed to read from MS Windows console");
	}

	auto converted_size = WideCharToMultiByte(CP_UTF8, 0, wbuffer, read, charbuffer, CBUFFER_SIZE, nullptr, nullptr);

	std::string result(charbuffer, converted_size);

	while (!result.empty() && (result.back() == '\r' || result.back() == '\n'))
	{
		result.pop_back();
	}

	dest = result;

	return input;
};

std::wstring platform_dependent_encoding(std::string source)
{
	std::wstring result(source.size(), wchar_t{});

	MultiByteToWideChar(CP_UTF8, 0, source.data(), source.size(), result.data(), result.size());

	return result;
}

#else

auto crossplatform_getline = [] (std::istream &input, std::string &dest) -> std::istream &
{
	return std::getline(input, dest);
};

std::string platform_dependent_encoding(std::string source)
{
	return source;
}

#endif

#include "simple_tests.h"
#include "huge_test.h"
#include "dictionary_manager.h"

// DO NOT!!! Don't test web lookup on huge files because this leads to 403 error and abortion of application
// TO-DO:	check HTTP status before procceeding with any presumable response content
//		provide error messages in case of unsuccessfull response status upon lookup attempts	DONE
//
//
// Windows TO-DO:	Teach Windows to open files with spaces (\screening suffices) and unicode characters in path DONE
// Windows-2 TO-DO:	Reimplement former feature via UTF-16 encoded wstrings	DONE
//
// Architecture TO-DO:	ensure inner consistency of classes DONE
// 				complete inner reimplementation debt TO-DOs DONE
// 				move type aliases to separate dictionary_types.h header DONE
// 			rewrite stored types from variable Entries to shared_ptr<Entry>	DONE
// 				reimplement all functions dealing with Entry		DONE
// 				make Entry type polymorphic (add virtual destructor)	DONE
// 				provide custom sort with one more possible ComparisonType::Custom			DONE
// 				create one more custom sort overload accepting template Comp if ::Custom		DONE
// 				wrap that custom in another lambda dealing with results of dynamic_cast<> failure	DONE
// 			find every place that is creating an Entry and make it template, accepting CustomEntry type with Entry as default	DONE
//			check if necessity in const remains after turn to pointer storing, get rid of it if possible				DONE
//
// TDD Routine TO-DO:	write tests for the interface features DONE
//			provide all the interface features developed DONE
//			when all beforementioned works consider interface development DONE
//			write a huge complex test involving all of those features DONE
//			check its work on two languages: english and russian DONE
//
// Considerable optimization TO-DO:	provide DefaultEntrySorter with transparent comparator overloads accepting string_view
// 					use that for the lookup features
// 					get a rule-of-thumb measurements of the effectiveness impact using HUGE TEST	DONE: 1/6 faster now
//
// Important feature TO-DO:		provide user-supplied string parsing to the dictionary	DONE
//
// Serialization TODO:			rewrite insides using the boost data structure includes		DONE
// 					add template serialize functions				DONE
// 					link boost where necessary					DONE
//
// 					add to the DictionaryManager following functions:
//
// 					.rename(string)			to name the dictionary, stored in member string			|
// 					.get_name() const		returns the dictionary name					| DONE
// 					.save_dictionary() const	saves dictionary under current name (default one available)	|
//
// 					add the namespace-scoped function that accepts the file name and returns DictionaryManager object
// 					DictionaryManager load_dictionary(file name)		DONE
// 					DictionaryManager load_dictionary(ifstream)		DONE
// 					in case of failure throws runtime errors		DONE
//
// 					for further user convenience provide a function returning list of all possible dictionaries	|
// 					std::vector<std::string> available_dictionaries()						|
// 					consider returning a pair of load_dictionary-required filename and user-readable filename	|
// 					possible type is std::pair<string, string_view> or custom struct instead of the std::pair	| DONE
//
// 					design some tests to check the correctness of the abovementioned		DONE
// 					write tests as parts of huge_test						DONE
// 					check the correctness using both english and russian version
//
// Refinement TODO:	make load_dictionary return the DictionaryManager objects with names corresponding to requested ones
//
// Premature optimization TODO: 	consider using custom allocation for storing dictionary
// 					deduce required space upon dictionary loading
// 					how to resize when more space is needed during the work?
// 					what if there is not enough allocated space?
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
	std::cout << "Write full path of file to add it to dictionary\n> " << std::flush;

	std::string path;
	crossplatform_getline(std::cin, path);

	std::ifstream file(platform_dependent_encoding(path));
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
	dm.define(longest_7);

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
	auto program_title = u8"DictionaryCreator 0.8.3";
	std::cout << program_title << std::endl;
#ifdef _WIN32
	if (!UTF8_awareness_for_windows_console)
	{
		std::cout << "Warning: Windows console output isn't UTF-8 aware!" << std::endl;
	}
#endif

	if (argc == 2)
	{
		if (!strncmp(argv[1], "TEST", 4))
		{
			simple_tests::tests(program_title);
		}
		
		if (!strncmp(argv[1], "HUGETEST", 8))
		{
			huge_test::run_all_tests();
		}
	}
	else
	{
		manage_dictionary();
	}

	return 0;
}
