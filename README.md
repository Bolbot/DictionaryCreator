# DictionaryCreator
A simple library that creates a dictionary out of user-specified text files. 
Supports definition loading, different criteria sorting, serialization, and customizable entry type for storing words.

## Requirements
You will only need C++17 compliant compiler and **Conan 2**. Conan will do the rest. It's going to pull these dependencies:
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
  
## How to build

Clone the repository and open it in your terminal

```sh
conan install . --build=missing¹
cmake --preset conan-release²
cmake --build --preset conan-release
```

1. If you haven't used conan before, you need to run `conan profile detect` first. It shall create a default profile based on your system configuration.


2. Windows uses `conan-default`, Linux uses `conan-release`.<br/>Depending on your CMake version, you might need to use a different command here. Check the output for these lines:

<span style="font-family:monospace;color:grey;background-color:">
&emsp;&emsp;&emsp;&emsp;conanfile.txt: Generator 'CMakeToolchain' calling 'generate()'<br/>
&emsp;&emsp;&emsp;&emsp;conanfile.txt: CMakeToolchain generated: conan_toolchain.cmake<br/>
<span style="color:teal">&emsp;&emsp;&emsp;&emsp;conanfile.txt: CMakeToolchain: Preset 'conan-default' added to CMakePresets.json.<br/>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;(cmake>=3.23) cmake --preset conan-default<br/>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;(cmake<3.23) cmake path -G "Visual Studio 17 2022" <nobr>-DCMAKE_TOOLCHAIN_FILE=generators\conan_toolchain.cmake</nobr>  -DCMAKE_POLICY_DEFAULT_CMP0091=NEW<br/>
</span></span>

### Additional features

These features are optional and disabled by default. To enable them, either set the checkboxes in your _CMake (cmake-gui)_ or manually edit _CMakeCache.txt_ in your build directory, setting them to ON:
* BUILD_TESTING enables unit tests for dictionary creator library (requires Boost)
* INSTALL_AND_PACKAGE creates installers and prepares dictionary creator for export as a CMake target
* SERIALIZATION enables the serialization, i.e. saving/loading the dictionaries from the file system

### Troubleshooting

If it skips the build of some binaries or starts to print obscure messages, try the following steps:

* Delete CMakeUserPresets.json from your source directory
* Delete the entire *build* directory
* Rerun the whole building routine
