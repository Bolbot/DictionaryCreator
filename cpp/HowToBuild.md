## How to build

Clone the repository and open it in your terminal.

Navigate to the cpp/ directory. Next commands assume execution from the same directory as this instruction file.

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
