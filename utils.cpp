#include "utils.h"

char lowercase_letter(char c)
{
	return tolower(c);
}
wchar_t lowercase_letter(wchar_t w)
{
	return towlower(w);
}

#if defined __STRINGTYPE_IS_WSTRING__ && __STRINGTYPE_IS_WSTRING__
std::wostream &output = std::wcout;
std::wistream &input = std::wcin;
std::wostream &error_output = std::wcerr;
#else
std::ostream &output = std::cout;
std::istream &input = std::cin;
std::ostream &error_output = std::cerr;
#endif