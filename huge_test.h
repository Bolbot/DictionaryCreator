#ifndef __HUGE_TEST_H__
#define __HUGE_TEST_H__

#include "dictionary_manager.h"
#include <set>
#include <string>
#include <vector>
#include <functional>
#include <chrono>

namespace huge_test
{
	void dm_creation();

	void special_cases();

	void main_huge_test_english();

	void main_huge_test_russian();

	void run_all_tests();

	template <typename Lambda, typename TimeUnit = std::chrono::milliseconds>
	auto execution_time(Lambda wrapped_task)
	{
		auto before = std::chrono::steady_clock::now();
		wrapped_task();
		auto after = std::chrono::steady_clock::now();

		return std::chrono::duration_cast<TimeUnit>(after - before);
	}
}

#endif
