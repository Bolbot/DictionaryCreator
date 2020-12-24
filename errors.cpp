Scanning dependencies of target regex_parser
[ 10%] Building CXX object CMakeFiles/regex_parser.dir/regex_parser.cpp.o
/home/whera/c_cpp/projects/DictionaryCreator/regex_parser.cpp: In constructor ‘pcre_parser::RegexParser::RegexParser(const char*)’:
/home/whera/c_cpp/projects/DictionaryCreator/regex_parser.cpp:10:9: error: invalid operands of types ‘const char*’ and ‘const char [44]’ to binary ‘operator<<’
   10 |   error << "Compilation of regular expression failed:\n\t" << pattern
      |   ~~~~~ ^~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      |   |        |
      |   |        const char [44]
      |   const char*
/home/whera/c_cpp/projects/DictionaryCreator/regex_parser.cpp:13:29: error: ‘error_mesage’ was not declared in this scope; did you mean ‘error_message’?
   13 |    throw std::runtime_error(error_mesage.str());
      |                             ^~~~~~~~~~~~
      |                             error_message
CMakeFiles/regex_parser.dir/build.make:81: recipe for target 'CMakeFiles/regex_parser.dir/regex_parser.cpp.o' failed
make[2]: *** [CMakeFiles/regex_parser.dir/regex_parser.cpp.o] Error 1
CMakeFiles/Makefile2:121: recipe for target 'CMakeFiles/regex_parser.dir/all' failed
make[1]: *** [CMakeFiles/regex_parser.dir/all] Error 2
Makefile:102: recipe for target 'all' failed
make: *** [all] Error 2
