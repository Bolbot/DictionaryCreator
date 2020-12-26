#ifndef __CONNECTIONS_H__
#define __CONNECTIONS_H__

#include "utils.h"

#define USING_CURL_FOR_CONNECTIONS 1

#if __has_include(<curl/curl.h>) && USING_CURL_FOR_CONNECTIONS
#include <curl/curl.h>
#define __CURL_IS_AVALIABLE__
#endif


namespace connections
{
	StringType get(const StringType &URI);

	StringType lookup_online_dictionary(const StringType &word);

	StringType lookup_online_dictionary(const WordType &word);
}


#endif
