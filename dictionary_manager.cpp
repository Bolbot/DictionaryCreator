#include "dictionary_manager.h"

dictionary_creator::DictionaryManager::DictionaryManager(dictionary_creator::Language language) :
	definer{ [language] (dictionary_creator::utf8_string word) { return define_word(std::move(word), language); } },
	dictionary{ language },
	creator{ language },
	exporter{ std::cout, u8"----" }
{
	srand(time(nullptr));
}

void dictionary_creator::DictionaryManager::add_input_file(std::ifstream &&input_stream)
{
	creator.add_input(std::move(input_stream));
}

void dictionary_creator::DictionaryManager::add_input_file(std::string file_name)
{
	std::ifstream stream(file_name);
	if (stream.good())
	{
		creator.add_input(std::move(stream));
	}
}

void dictionary_creator::DictionaryManager::parse_all_pending()
{
	dictionary.merge(creator.parse_to_dictionary());
}

void dictionary_creator::DictionaryManager::parse_one_line(dictionary_creator::utf8_string line)
{
	dictionary.merge(creator.parse_line(std::move(line)));
}

std::shared_ptr<dictionary_creator::Entry> dictionary_creator::DictionaryManager::lookup_or_add_word(dictionary_creator::utf8_string word)
{
	if (auto ptr = dictionary.lookup(word); ptr)
	{
		return ptr;
	}

	dictionary.add_word(word);

	return dictionary.lookup(word);
}

bool dictionary_creator::DictionaryManager::contains_word(dictionary_creator::utf8_string word) const
{
	return (dictionary.lookup(word) != nullptr);
}

dictionary_creator::subset_t dictionary_creator::DictionaryManager::get_subset(dictionary_creator::ComparisonType criterion, size_t number) const
{
	return dictionary.get_top(criterion, number);
}

dictionary_creator::subset_t dictionary_creator::DictionaryManager::get_subset(dictionary_creator::letter_type letter, size_t number) const
{
	auto res = dictionary.get_letter_entries(letter);

	if (number != 0 && res.size() > number)
	{
		res.resize(number);
	}

	return res;
}

dictionary_creator::subset_t dictionary_creator::DictionaryManager::get_undefined(size_t number) const
{
	auto res = dictionary.get_undefined();

	if (number != 0 && res.size() > number)
	{
		res.resize(number);
	}

	return res;
}

std::shared_ptr<dictionary_creator::Entry> dictionary_creator::DictionaryManager::get_random_word() const
{
	return dictionary.get_random_word();	
}

dictionary_creator::subset_t dictionary_creator::DictionaryManager::get_random_words(size_t number) const
{
	return dictionary.get_random_words(number);
}

std::shared_ptr<dictionary_creator::Entry> dictionary_creator::DictionaryManager::define(dictionary_creator::utf8_string word)
{
	auto entry = lookup_or_add_word(word);

	entry->define(definer);

	return entry;
}

std::shared_ptr<dictionary_creator::Entry> dictionary_creator::DictionaryManager::define(std::shared_ptr<dictionary_creator::Entry> word)
{
	word->define(definer);

	return word;
}

std::shared_ptr<dictionary_creator::Entry> dictionary_creator::DictionaryManager::define(const dictionary_creator::Entry &word)
{
	auto res = lookup_or_add_word(word.get_word());

	res->define(definer);

	return res;
}

const dictionary_creator::subset_t &dictionary_creator::DictionaryManager::define(const dictionary_creator::subset_t &entries)
{
	for (const auto &i: entries)
	{
		i->define(definer);
	}

	return entries;
}

void dictionary_creator::DictionaryManager::set_output(std::ostream &output_stream)
{
	exporter = DictionaryExporter(output_stream, undefined_warnings[static_cast<size_t>(dictionary.get_language())]);
}

void dictionary_creator::DictionaryManager::export_dictionary(dictionary_creator::ExportOptions options)
{
	exporter.export_dictionary(dictionary, options);
}

void dictionary_creator::DictionaryManager::export_top(dictionary_creator::ComparisonType criterion, size_t top, dictionary_creator::ExportOptions options)
{
	dictionary_creator::ExportOptions additional;
	switch (criterion)
	{
	case dictionary_creator::ComparisonType::MostFrequent:
	case dictionary_creator::ComparisonType::LeastFrequent:
		additional = dictionary_creator::ExportOptions::Frequency;
		break;
	case dictionary_creator::ComparisonType::Longest:
	case dictionary_creator::ComparisonType::Shortest:
		additional = dictionary_creator::ExportOptions::Length;
		break;
	case dictionary_creator::ComparisonType::MostAmbiguous:
	case dictionary_creator::ComparisonType::LeastAmbiguous:
		additional = dictionary_creator::ExportOptions::Ambiguousness;
		break;
	default:
		throw std::runtime_error("Unexpected comparison criterion");
	};
	exporter.export_entries(dictionary.get_top(criterion, top), options | additional);
}

void dictionary_creator::DictionaryManager::export_subset(const dictionary_creator::subset_t &subset, dictionary_creator::ExportOptions options)
{
	exporter.export_entries(subset, options);
}

void dictionary_creator::DictionaryManager::rename(dictionary_creator::utf8_string new_name)
{
	name = std::move(new_name);
}

dictionary_creator::utf8_string dictionary_creator::DictionaryManager::get_name() const noexcept
{
	return name;
}

void dictionary_creator::DictionaryManager::save_dictionary() const
{
	if (std::filesystem::exists(dictionaries_directory) == false && std::filesystem::create_directory(dictionaries_directory) == false)
	{
		throw std::runtime_error("Saving dictionaries is unavailable");
	}

	std::ofstream output(dictionary_creator::utf8_string{ dictionaries_directory } + u8"/" + name + dictionaries_extension);
	if (output.good())
	{
		boost::archive::text_oarchive oa(output);
		oa & dictionary;
	}
	else
	{
		throw std::runtime_error("Saving failed, coudn't write to that file");
	}
}

dictionary_creator::DictionaryManager dictionary_creator::load_dictionary(dictionary_creator::utf8_string dictionary_filename)
{
	dictionary_creator::Dictionary acquired(dictionary_creator::Language::English);		// TODO: get rid of english-initialization clumsiness

	if (std::ifstream stream(dictionary_filename); stream.good())
	{
		boost::archive::text_iarchive ia(stream);
		ia & acquired;
	}
	else
	{
		throw std::runtime_error("Failed to read the file");
	}

	dictionary_creator::DictionaryManager result(acquired.get_language());
	result.dictionary = std::move(acquired);
	return result;
}

dictionary_creator::DictionaryManager dictionary_creator::load_dictionary(std::ifstream &&dictionary_file)
{
	dictionary_creator::Dictionary acquired(dictionary_creator::Language::English);		// TODO: get rid of english-initialization clumsiness

	if (dictionary_file.good())
	{
		boost::archive::text_iarchive ia(dictionary_file);
		ia & acquired;
	}
	else
	{
		throw std::runtime_error("Failed to read the file");
	}

	dictionary_creator::DictionaryManager result(acquired.get_language());
	result.dictionary = std::move(acquired);
	return result;
}

std::vector<dictionary_creator::dictionary_filename> dictionary_creator::available_dictionaries()
{
	std::vector<dictionary_creator::dictionary_filename> result;

	for (const auto &path: std::filesystem::directory_iterator(dictionary_creator::DictionaryManager::dictionaries_directory))
	{
		auto full = path.path().string();
		if (auto found = full.rfind(dictionary_creator::DictionaryManager::dictionaries_extension);
				found == (full.size() - strlen(dictionary_creator::DictionaryManager::dictionaries_extension)))
		{
			dictionary_creator::dictionary_filename current = { std::move(full), std::string_view{} };
			size_t start_shift = strlen(dictionary_creator::DictionaryManager::dictionaries_directory) + 1;
			size_t end_shift = current.full.rfind(dictionary_creator::DictionaryManager::dictionaries_extension);

			result.push_back(std::move(current));
			result.back().human_readable = std::string_view(result.back().full.data() + start_shift, end_shift - start_shift);
		}
	}

	return result;
}
