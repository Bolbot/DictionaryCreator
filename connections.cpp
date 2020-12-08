#include "connections.h"


#ifdef __THIS_IS_WINDOWS__
StringType connections::get(const StringType &URI)
{
	// TODO: implement https connection for Windows
	return "Not yet implemented for Windows";
}
#else
StringType connections::get(const StringType &URI)
{
	cpr::Response response = cpr::Get(cpr::Url{ URI }, cpr::VerifySsl(false));
	return response.text;
}
#endif

StringType connections::lookup_online_dictionary(const StringType &word)
{
	StringType dictionaryapi_request{ "https://api.dictionaryapi.dev/api/v2/entries/en/" };
	StringType exact_request_address = dictionaryapi_request + word;

	return connections::get(exact_request_address);
}
