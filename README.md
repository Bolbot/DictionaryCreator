# DictionaryCreator
A simple library that creates a dictionary out of user-specified text files. 
Supports definition loading, different criteria sorting, serialization, and customizable entry type for storing words.

## Requirements
In order to use this library you should have following:
  * [CURL](https://github.com/curl/curl)
  * [Boost](https://www.boost.org/users/download/)
  * [PCRE2](https://github.com/PCRE2Project/pcre2/)
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
  
## How to build

Clone the repository and open it in your terminal

```sh
cmake -S . -B build
cmake --build build --config Release --parallel 4
```

By default, it downloads all the necessary dependencies and leaves all the optional features in toggled off state.<br/>
The optional features that require dependencies are:
  * [CURL](https://github.com/curl/curl) for `dictionary_definer` to download the definitions, otherwise it's no-op
  * [Boost](https://www.boost.org/users/download/) for serialization between launches and unit tests, otherwise unavailable


To enable Inatlling and Packaging, make sure all dependencies are provided and set InstallAndPackage option to ON.
To enable Testing, make sure Boost unit_test_framework is abailable and set BUILD_TESTING option to ON.
For setting the options, either use CMake GUI or manually find them in CMakeCache.txt in your build directory once it's generated.