#include "connections.h"

#ifdef __CURL_IS_AVALIABLE__

struct CurlGlobalHandle
{
	CurlGlobalHandle()
	{
		if (curl_global_init(CURL_GLOBAL_ALL) != 0)
		{
			output << "Problems with curl global initialization. All further use of curl is undefined\n";
		}
		else
		{
			output << "Using curl for https access.\n";
		}
	}
	~CurlGlobalHandle()
	{
		curl_global_cleanup();
	}
} raii_handle_for_global_curl_initialization;

class CurlEasyHandle
{
public:
	static int writer(char *data, size_t size, size_t nmemb, StringType *destination)
	{
		if (destination == nullptr)
		{
			return 0;
		}
		size_t actual_size = size * nmemb;
		destination->append(data, actual_size);
		return actual_size;
	}
	explicit CurlEasyHandle(const char *address) : handle{ curl_easy_init() }, valid_state{ handle != nullptr }
	{
		auto setup_handle = [this] (auto option, auto &&value)
		{
			auto setopt_result = curl_easy_setopt(handle, option, std::forward<decltype(value)>(value));
			valid_state = (setopt_result == CURLE_OK);
		};

		if (valid_state)
		{
			setup_handle(CURLOPT_URL, address);
			setup_handle(CURLOPT_SSL_VERIFYHOST, 0L);
			setup_handle(CURLOPT_SSL_VERIFYPEER, 0L);
			setup_handle(CURLOPT_WRITEFUNCTION, writer);
			setup_handle(CURLOPT_WRITEDATA, &response);
		}

		if (valid_state)
		{
			valid_state = (curl_easy_perform(handle) == CURLE_OK);
		}
	}
	~CurlEasyHandle()
	{
		curl_easy_cleanup(handle);
	}

	StringType get_response() & noexcept
	{
		return response;
	}
	StringType get_response() && noexcept
	{
		return std::move(response);
	}
private:
	CURL *handle;
	bool valid_state;
	StringType response;
};

#else

class CurlEasyHandle
{
public:
	CurlEasyHandle(const char *str)
	{}
	StringType get_response() const noexcept
	{
		return "Curl is unavaliable\n";
	}
};

#endif


StringType connections::get(const StringType &URI)
{
	return CurlEasyHandle(URI.data()).get_response();
}

StringType connections::lookup_online_dictionary(const StringType &word)
{
	StringType dictionaryapi_request{ "https://api.dictionaryapi.dev/api/v2/entries/en/" };
	StringType exact_request_address = dictionaryapi_request + word;

	return connections::get(exact_request_address);
}
