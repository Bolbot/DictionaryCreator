#include "console_file_system.h"

fs::FSStringType fs::to_fsstring(fs::FSStringType str) noexcept
{
	return str;
}

#ifdef __THIS_IS_WINDOWS__
fs::FSStringType fs::to_fsstring(std::string str) noexcept
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}
#endif

std::string fs::stdstring(const std::string &s) noexcept
{
	return s;
}

#ifdef __THIS_IS_WINDOWS__
std::string fs::stdstring(const std::wstring &ws) noexcept
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(ws);
}
#endif

fs::FSStringType fs::directory;
std::set<fs::FSStringType> fs::text_files_extensions;

void fs::request_the_extensions(fs::FSStringType source)
{
	fs::directory = source;
	auto possible_extensions = fs::get_possible_extensions();

	output << "There are files of " << possible_extensions.size() << " extensions:\n";

	for (const auto &s : possible_extensions)
	{
		output << "\t" << fs::stdstring(s) << std::endl;
	}
	
#if __STRINGTYPE_IS_WSTRING__
	auto input_terminator_string = L"!";
#else
	auto input_terminator_string = "!";
#endif
	output << "Specify which of them to parse for the dictionary.\nExample: .txt or simply txt\n"
			<< "Use " << input_terminator_string << " to terminate input\n";

	StringType ext;
	//std::string ext;

	while (input >> ext && ext != input_terminator_string)
	{
		if (*ext.begin() != '.')
		{
			ext.insert(ext.begin(), '.');
		}
		text_files_extensions.insert(fs::to_fsstring(ext));
	}

	output << "Parsing files of following extensions:\n";
	for (const auto &s : text_files_extensions)
	{
		output << "\t" << fs::stdstring(s) << std::endl;
	}
}

std::set<fs::FSStringType> fs::get_possible_extensions()
{
	std::set<fs::FSStringType> extensions;
			
	for (auto &p : fs::directory_iterator(directory))
	{
		extensions.emplace(p.path().extension().c_str());
	}

	return extensions;
}

std::vector<FileInputStream> fs::provide_all_files_streams()
{
	std::vector<FileInputStream> streams;

	for (auto &p : fs::directory_iterator(fs::directory))
	{
		if (fs::text_files_extensions.find(p.path().extension()) != fs::text_files_extensions.end())
		{
			FileInputStream current_file(p.path());
			streams.push_back(std::move(current_file));
		}
	}

	return streams;
}

#if __STRINGTYPE_IS_WSTRING__
std::wofstream fs::output_to_file(fs::FSStringType file_name)
{
	return std::wofstream(file_name);
}
#else
std::ofstream fs::output_to_file(fs::FSStringType file_name)
{
	return std::ofstream(file_name);
}
#endif

fs::FSStringType fs::generate_sorted_file_name(fs::FSStringType prefix, size_t comparation_type)
{
	FSStringType space;
	FSStringType dot_txt;

#ifdef __THIS_IS_UNIX__
	static const std::vector<const char*> sorter_descriptions
	{
		"most frequent", "least frequent", "longest", "shortest", "most ambiguous", "least ambiguous"
	};
	space = " ";
	dot_txt = ".txt";
#elif defined __THIS_IS_WINDOWS__
	static const std::vector<const wchar_t*> sorter_descriptions
	{
		L"most frequent", L"least frequent", L"longest", L"shortest", L"most ambiguous", L"least ambiguous"
	};
	space = L" ";
	dot_txt = L".txt";
#endif

	FSStringType destination = std::move(prefix);
	const auto &sorting_type_description = sorter_descriptions[comparation_type];
	destination += space;
	destination += sorting_type_description;
	destination += dot_txt;

	return destination;
}