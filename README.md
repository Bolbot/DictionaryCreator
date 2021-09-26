# DictionaryCreator
A simple library that creates a dictionary out of user-specified text files. 
Supports definition loading, different criteria sorting, serialization, and customizable entry type for storing words.

## Requirements
In order to use this library you should have following:
  * [CURL](https://github.com/curl/curl)
  * [Boost](https://www.boost.org/users/download/)
  * [unofficial-pcre](https://github.com/albertliangcode/PCRE)
  * C++17 compliant compiler

This library works with UTF-8 encoded strings only. If you're using any letters beside the basic English alphabet be sure they're represented as UTF-8.
The same applies to text files. Not that .txt extension is required but contents should be encoded as UTF-8 (BOM is optional).
  
## Design priorities
Following virtues were chosen as more important while creating the library 
* Simplicity. Users should be able to start using this software quickly and painlessly without reading tons of documentation. 
Names of classes, member functions, arguments, etc. speak for themselves as much as possible.
* Maintainability. The initial design allowed to quickly expand core functionality and provide users with more features such as work with subsets. 
Different languages support is designed so that introducing a new language should be a matter of a few strings addition. 
This currently applies to left-to-right top-to-bottom scripts only.
* Stability. The design of the library is aimed at stability and reliability, no memory or other resource leaks, and no crashes.
In case of irregular input data or other unusual cases functions simply provide a user with error messages or empty results in the worst case. 
No exceptions are thrown and no processes are interrupted unless it's a problem of some OS-level nature, such as out of memory or permissions violation.
  
## Support

To get started check out the **First steps** of this readme. This is some minimal example to get you going.

For further more advanced actions consult with **Advanced usage** below. So far that is the most excessive description other than code itself.

If you still have any questions, feel free to ask them [right here on GitHub](https://github.com/Bolbot/DictionaryCreator/discussions/new)

	
## How to use

### First steps

1. Include dictionary_manager.h to access dictionary_manager class - your interface to all basic and most the advanced features.  
Most of the features under the hood are organized in `dictionary_creator` namespace so unless any of those names clash with any of yours 
you can put a using-directive or using-declaration in order to shorten the further code.
    ```cpp
    #include "dictionary_manager.h"
    
    // Less reliable, use for small projects only
    //using namespace dictionary_creator;   // brings dictionary_manager, dictionary_creator, dictionary_entry etc. to current scope
    
    // More reliable: each name clash is dealt with separately
    using dictionary_creator::DictionaryManager;
    using dictionary_creator::Language;
    ```
    
2. Create a `DictionaryManager` object. In the constructor you should specify the language of the dictionary you're creating.  
You can also specify the name of your dictionary as the second argument. This is optional. Be sure to keep non-English names UTF-8 encoded.
    ```cpp
    DictionaryManager my_great_dictionary(Language::English);  // default name
    DictionaryManager chekhov_kashtanka(Language::Russian, u8"Чехов - Каштанка");  // custom name
    ```
    
3. Throw in some input files. This is done via `add_input_file` member function which accepts either `std::ifstream` objects 
or filenames in absolute or relative form. Latter means `std::string` or `std::wstring` so if you have `const char *` be sure to wrap it in `std::string{}`
    ```cpp
    my_great_dictionary.add_input_file(std::string{ "text example1.txt" });
    chekhov_kashtanka.add_input_file(std::string{ "Чехов - Каштанка.txt"});
    // or
    std::ifstream text_file("text example2.txt");
    my_great_dictionary.add_input_file(std::move(text_file));   // std::ifstream isn't copyable
    ```
    
4. Parse those files with `parse_all_pending` member function. This is where most under-the-hood processing happens. 
Proper nouns are stripped off as well as words shorter than minimal length (for English this length is 3 characters). 
Then words are stored by letters just like in any dictionary.
    ```cpp
    my_great_dictionary.parse_all_pending();
    chekhov_kashtanka.parse_all_pending();
    ```
    
5. Export your dictionary to the text file. First, specify output stream via `set_output` member function. 
Then call `export_dictionary` member function without arguments to fill that file with your dictionary.
Output can be customized with special arguments to `export_dictionary` but the default set of arguments should be optimal for most uses.
    ```cpp
    std::ofstream output_file("dictionary_test_results.txt");
    my_great_dictionary.set_output(output_file);
    my_great_dictionary.export_dictionary();          // default export should be good enough
    
    std::ofstream output_file_2("chekhov_results.txt");
    chekhov_kashtanka.set_output(output_file_2);
    chekhov_kashtanka.export_dictionary();
    ```

6. See the output files you've specified in the previous step. They should contain dictionaries. 
If they don't, looks like you didn't have `text example1.txt`, `text example 2.txt`, and `Чехов - Каштанка.txt` in the directory you ran the program from.


### Advanced usage 

Following functions and features are currently available


#### Quick parsing of text

For small amounts of text such as one line or a paragraph occupying one line there is `parse_one_line` 
member function of `DictionaryManager` that is accepting UTF-8 encoded std::strings
  ```cpp
  DictionaryManager dm(Language::English);
  dm.parse_one_line(u8"The quick brown fox jumps over the lazy dog");   // u8"" is optional for english but required otherwise
  ```
This way no stream objects are created and unnecessary overhead can be avoided if you need a simple line parsed to a dictionary.

#### Lookup

To do the quick check whether the word is already in the dictionary use `contains_word` member function that returns `bool`, 
true if the word is contained or false if not.

All the word entries are kept inside as objects of `DictionaryEntry` class. They can be accessed as `std::shared_ptr<DictionaryEntry>` via `lookup_or_add_word` function.
The result is always guaranteed to be the shared pointer to an existing entry in the dictionary. If that word was present beforehands, it's acquired. 
Otherwise the entry is created. The difference relates to all the additional the content that is kept along with word (see below).

#### Subsets

DictionaryManager supports a wide range of entry subsets operations. 
Every `get_` function except for `get_random_` supports `number` argument that is represented as `size_t` and for most functions is optional. 
`number` is used to specify the maximum words retrieved. If that exceeds the actual number of words, all related entries are retrieved.

To get all words starting from a given letter use `get_subset` with that letter as argument. Letter should be a UTF-8 encoded std::string.
  ```cpp
  dm.get_subset(std::string{ "A" });              // all letters starting with a
  dm_fra.get_subset(std::string{ u8"é" }, 3);     // at most 3 letters starting with é (notice u8"")
  ```
  
To get the subset sorted by some criterion use `get_subset` with that criterion as an argument. Following criteria are available
* MostFrequent
* LeastFrequent
* Longest
* Shortest
* MostAmbiguous
* LeastAmbiguous

With no second argument that means all dictionary entries are sorted by respective criterion.

They are accessed as ComparisonType::
  ```cpp
  using dictionary_creator::CopmarisonType;
  dm.get_subset(ComparisonType::Longest, 20);       // up to 20 longest words
  dm.get_subset(ComparisonType::LeastFrequent, 2);  // two least frequent words
  dm.get_subest(ComparisonType::MostAmbiguous);     // all entries sorted from most ambiguous to least ambiguous  
  ```

To get all undefined letters use `get_undefined`. The number of undefined letters retrieved can be capped with an optional `size_t number` argument.

To get one random word use `get_random_word`. To get n random words use `get_random_words(n)` where type of n is `size_t` and 
if n exceeds the actual number of entries, all entries are returned in random order.

#### Definition

Definition is done via `define` member function of DictionaryManager. Is accepts words in form of `std::string`, 
`dictionary_creator::DictionaryEntry`, or `std::shared_ptr<dictionary_creator::DictionaryEntry>` as well as in subsets
that are retrieved by `get_subset`, `get_undefined`, and `get_random_words` functions.

#### Serialization

Every dictionary can be serialized that is stored in your filesystem for further work. 
Each dictionary should have a unique name so be sure to give your dictionary a unique name or at least rename it before saving.
You can check the current name via `get_name` member function. You can change it using `rename` function.  

```cpp
{
	DictionaryManager king(Language::English, "Stephen King");
	std::cout << king.get_name();			// prints Stephen King
	king.rename("modern sci-fi");
	king.save_dictionary();						// saved as modern sci-fi
}
```
		
To load dictionary you can call `load_dictionary` with the absolute path to the file saved in your filesystem. 
To facilitate that `available_dictionaries` function can provide you with `std::vector` of custom struct having two members:
* `human_readable` is for you
* `full` is for `load_dictionary`
Be sure to use only full names with `load_dictionary` member function.

```cpp
{
	auto all_dictionaries = available_dictionaries();
	// ...
	// choose your dictionary, say it was at [3]
	// ...
	auto modern_scifi = load_dictionary(all_dictionaries[3].full)
}
```

#### Custom entry types and sorting criteria

Custom entry types are available to store entries in the dictionary. You type must inherit `dictionary_creator::DictionaryEntry` class. 
Be sure to check dictionary_entry.h to get familiar with its interface. 
To add word use the template version of `lookup_or_add_word` parametrized with your type name and provide all arguments right after the word itself.

A custom comparator can be passed to `get_subset`. It must be a binary predicate accepting two entries by a `const &`. 
Entry types can be both `DictionaryEntry` as used by default and custom entry types.
