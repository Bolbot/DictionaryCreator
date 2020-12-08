#include "utils.h"

std::ostream &output = std::cout;
std::istream &input = std::cin;


FSStringType to_fsstring(FSStringType fs) noexcept
{
	return fs;
}

#ifdef __THIS_IS_WINDOWS__
FSStringType to_fsstring(std::string str) noexcept
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}
#endif

std::string stdstring(const std::string &s) noexcept
{
	return s;
}

#ifdef __THIS_IS_WINDOWS__
std::string stdstring(const std::wstring &ws) noexcept
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(ws);
}
#endif
