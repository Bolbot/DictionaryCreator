#include "dictionary_entry.h"

dictionary_creator::Entry::Entry(dictionary_creator::utf8_string word)
	: word{ std::move(word) }, encounters{ 1 }, defined{ false }
{}

dictionary_creator::utf8_string dictionary_creator::Entry::get_word() const noexcept
{
	return word;
}

const dictionary_creator::definitions_t &dictionary_creator::Entry::get_definitions() const noexcept
{
	return definitions;
}

bool dictionary_creator::Entry::is_defined() const noexcept
{
	return defined;
}

const dictionary_creator::Entry &dictionary_creator::Entry::define(const dictionary_creator::definer_t &definer)
{
	if (!defined)
	{
		definitions = definer(word);

		if (definitions.size() > 0)
		{
			defined = true;
		}
	}

	return *this;
}

size_t dictionary_creator::Entry::get_counter() const noexcept
{
	return encounters;
}

void dictionary_creator::Entry::increment_counter(size_t i) noexcept
{
	encounters += i;
}

dictionary_creator::Entry::operator const char *() const noexcept
{
	return word.c_str();
}

dictionary_creator::Entry::~Entry() = default;

/*
bool dictionary_creator::operator<(const dictionary_creator::Entry &a, const dictionary_creator::Entry &b)
{
	return a.get_word() < b.get_word();
}
*/