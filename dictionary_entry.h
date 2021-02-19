#ifndef __DICTIONARY_ENTRY_H__
#define __DICTIONARY_ENTRY_H__

#include <functional>
#include <string>
#include <map>
#include <set>

#include "dictionary_types.h"

namespace dictionary_creator
{
	// every member function is declared const because std::set.find returns const iterators
	// if there is any wiser solution not involving indirection, it should be taken into consideration

	class Entry
	{
	public:
		explicit Entry(utf8_string word);

		utf8_string get_word() const noexcept;
		const definitions_t &get_definitions() const noexcept;

		bool is_defined() const noexcept;
		const Entry &define(const definer_t &definer) const;

		size_t get_counter() const noexcept;
		void increment_counter(size_t i = 1) const noexcept;

		operator const char *() const noexcept;
	private:
		const utf8_string word;
		mutable definitions_t definitions;
		mutable size_t encounters;
		mutable bool defined;
	};

	bool operator<(const Entry &a, const Entry &b);
}

#endif
