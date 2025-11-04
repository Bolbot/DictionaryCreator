# DictionaryCreator
A simple library that creates a dictionary out of user-specified text files. 
Supports definition loading, different criteria sorting, serialization, and customizable entry type for storing words.

## C++ to Rust transition
The project is being transitioned from C++ to Rust. The C++ part is in **cpp/** directory, and the Rust part will be in **rust/** directory.

## Requirements
With C++ project, you will only need C++17 compliant compiler and **Conan 2**. Conan will do the rest. It's going to pull these dependencies:
  * [**PCRE2**](https://github.com/PCRE2Project/pcre2/) for parsing the files
  * [CURL](https://github.com/curl/curl) for downloading the definitions
  * [Json](https://github.com/nlohmann/json) for parsing the downloaded definitions
  * [Boost](https://www.boost.org/users/download/) for unit tests and serialization

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
