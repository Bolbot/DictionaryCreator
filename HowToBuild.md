# Dependencies

For properly working, this project requires
  * [CURL](https://github.com/curl/curl)
  * [Boost](https://www.boost.org/users/download/)
  * [unofficial-pcre](https://github.com/albertliangcode/PCRE)
  * C++17 compliant compiler


## Optional features

### CURL

Without curl, the connections library becomes no-op and dictionary_definer becomes effectively useless.

### Boost

Without boost, the serialization is not available, so we won't be able to store artifacts between launches.
Additionally, boost is used for testing, so the tests won't be available either.

## Mandatory features

### Perl-Compatible Regular Expressions

Core facility of dictionary_creator library. Without PCRE, the whole application is useless.

### C++11 compliant compiler

Some code depends on C++17 syntactic sugar. In the future, it will also use C++17 features.