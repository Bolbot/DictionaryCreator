#include <iostream>

#include "dictionary_manager.h"

int main()
{
	dictionary_creator::DictionaryManager dictionary(dictionary_creator::Language::English);

	std::cout << "Pseudomain runs\n";

	return 0;
}
