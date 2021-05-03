#ifndef __SIMPLE_TESTS_H__
#define __SIMPLE_TESTS_H__

#include <iostream>
#include <string>

#include "dictionary.h"
#include "connections.h"
#include "dictionary_entry.h"
#include "dictionary_creator.h"
#include "dictionary_manager.h"

namespace simple_tests
{
	template <typename ... Args>
	constexpr auto arbitrary_sum(Args &&... args)
	{
		return (std::forward<Args>(args) + ...);
	}

	bool isprintable(char character) noexcept;

	void tests(const char *program_name);
	
	void test_connections();

	void test_entry();

	void test_dictionary();

	void test_dictionary_creator();

	void test_dictionary_exporter();

	void test_dictionary_manager();

	void test_custom_entry_type();

	void test_letter_related_features();

	void test_one_line_parser();

	void test_serialization();

}

#endif
