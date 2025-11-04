#define BOOST_TEST_MODULE Connections Regress Test
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "connections.h"

BOOST_AUTO_TEST_CASE(connections_alltogether)
{
	const std::string html_doc_start{ "<!doctype html>" };
	const std::string html_doc_end{ "</html>" };
	const std::string http_0{ "HTTP 0 " };
	const std::string http_404{ "HTTP 404 " };

	BOOST_TEST_INFO("get for valid http address yields html document");
	auto http_connect_res = connections::get("http://www.google.com/");
	BOOST_TEST_CHECK((boost::iequals(http_connect_res.substr(0, html_doc_start.size()), html_doc_start) &&
			boost::iequals(http_connect_res.substr(http_connect_res.size() - html_doc_end.size()), html_doc_end)));

	BOOST_TEST_INFO("get for valid https address yields html document");
	auto https_connect_res = connections::get("https://dictionaryapi.dev/");
	BOOST_TEST_CHECK((boost::iequals(https_connect_res.substr(0, html_doc_start.size()), html_doc_start) &&
			boost::iequals(https_connect_res.substr(https_connect_res.size() - html_doc_end.size()), html_doc_end)));

	BOOST_TEST_INFO("get for invalid https address yields HTTP 0 response");
	auto nonexistent_res = connections::get("https://thi.s.page.does.not.exist/");
	BOOST_TEST_CHECK(nonexistent_res.substr(0, http_0.size()) == http_0);

	BOOST_TEST_INFO("get for nonexistent https page yields HTTP 404 response");
	auto notfound_res = connections::get("https://google.com/this/page/cann/ot.befound");
	BOOST_TEST_CHECK(notfound_res.substr(0, http_404.size()) == http_404);
}
