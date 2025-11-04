#pragma once

#include "dictionary_types.h"

#ifndef BOOST_UNAVAILABLE
#include <boost/serialization/access.hpp>
#endif

namespace dictionary_creator
{
	class Entry
	{
	public:
		explicit Entry(utf8_string word = utf8_string{ "-" });

		utf8_string get_word() const noexcept;
		const definitions_t &get_definitions() const noexcept;

		bool is_defined() const noexcept;
		const Entry &define(const definer_t &definer);

		size_t get_counter() const noexcept;
		void increment_counter(size_t i = 1) noexcept;

		operator const char *() const noexcept;
		virtual ~Entry();
	private:
		utf8_string word;
		definitions_t definitions;
		size_t encounters;
		bool defined;

#ifndef BOOST_UNAVAILABLE
		friend class boost::serialization::access;
#endif

		template <typename A>
		void serialize(A &arch,[[ maybe_unused ]] const unsigned int version)
		{
			arch & word;
			arch & definitions;
			arch & encounters;
			arch & defined;
		}
	};
}
