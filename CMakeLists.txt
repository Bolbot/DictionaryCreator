cmake_minimum_required(VERSION 3.24)

project(DictionaryCreator
	VERSION 0.8.11
	DESCRIPTION "Soulution for parsing texts into dictionaries"
	LANGUAGES CXX)


# =========== PREPATIONS ===========

# in-source build prevention
if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
	message(FATAL_ERROR [[
In-source build attempted. You may delete CMakeCache.txt and CMakeFiles directory now.
To generate build for ${PROJECT_NAME} run
	cmake -S <source directory> -B <build directory>
Note source and build directories shouldn't match. If that is your deliberate intention,
edit CMakeLists.txt disabling this block of code.]])
endif()

set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_RELWITHDEBINFO_POSTFIX _rd)

# conventional files distribution
include(GNUInstallDirs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

# default unspecified CMAKE_BUILD_TYPE to DEBUG
get_property(multiconfig_generator GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)	# CMake 3.9
if (NOT multiconfig_generator AND NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type specified. Applying Debug.")
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Applying default build type" FORCE)
endif()

# shared libraries
option(BUILD_SHARED_LIBS "Build libraries as shared (.so in UNIX and .dll in Windows)" OFF)
if (${BUILD_SHARED_LIBS} AND ${CMAKE_SYSTEM_NAME} STREQUAL Windows)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# folders for MSVC
if (CMAKE_GENERATOR MATCHES "Visual Studio")
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()


# =========== DECLATATIONS ===========

configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}/config.h" @ONLY)

# interface target for compilation flags and properties
add_library(DictionaryCreator_compiler_flags INTERFACE)
target_compile_features(DictionaryCreator_compiler_flags INTERFACE cxx_std_17)
set(gcc_compiler_cpp "$<COMPILE_LANG_AND_ID:CXX,AppleClang,ARMClang,Clang,GNU>")
set(mvs_compiler_cpp "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(DictionaryCreator_compiler_flags
			INTERFACE
				$<${gcc_compiler_cpp}:-Wall;-Wextra;-Werror;-Wpedantic>
				$<${mvs_compiler_cpp}:-W4;-WX;/utf-8>)
target_compile_definitions(DictionaryCreator_compiler_flags INTERFACE $<${mvs_compiler_cpp}:_UNICODE>)

# core of the project in this subdirectory
add_subdirectory(dictionary_creator_library)

# filesystem for console application
add_subdirectory(filesystem_library)

# tests
option(BUILD_TESTING "Build tests for ${PROJECT_NAME}" OFF)
if ((${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME} OR ${PROJECT_NAME}_BUILD_TESTING) AND BUILD_TESTING)
	message(STATUS "Building tests for project ${PROJECT_NAME} in ${CMAKE_PROJECT_NAME}")
	enable_testing()
	add_subdirectory(tests)
endif()

# console application to run dictionary creator library
add_executable(DictionaryCreatorConsoleApp main.cpp $<$<PLATFORM_ID:Windows>:filesystem_library/windows/win_resource_file.rc>)
target_link_libraries(DictionaryCreatorConsoleApp PRIVATE DictionaryCreator_compiler_flags dictionary_manager console_fs_manager)
target_include_directories(DictionaryCreatorConsoleApp PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}")
set_target_properties(DictionaryCreatorConsoleApp PROPERTIES
	INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}:${CMAKE_INSTALL_LIBDIR}")

# minimalistic DictionaryCreator user for simple checks
#add_executable(pseudomain pseudomain.cpp)
#target_link_libraries(pseudomain PRIVATE DictionaryCreator_compiler_flags dictionary_manager)

option(IncludeWhatYouUseCheck "Leverage include-what-you-use feature to inspect the build for redundant inclusions" OFF)
find_program(iwyu_path NAMES include-what-you-use iwyu)
if (iwyu_path AND ${IncludeWhatYouUseCheck})
	message(STATUS "Setting CXX_INCLUDE_WHAT_YOU_USE property for DictionaryCreatorConsoleApp")
	set_property(TARGET DictionaryCreatorConsoleApp PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
	message(STATUS "Set to ${iwyu_path}")
elseif (${IncludeWhatYouUseCheck})
	message(WARNING "Cannot find Include What You Use program.")
endif()

find_program(GRAPHVIZ_DOT_EXECUTABLE NAMES dot)

if (GRAPHVIZ_DOT_EXECUTABLE)
# build graph generation
	add_custom_target(graph_generation ALL
		COMMAND	${CMAKE_COMMAND} -S ${CMAKE_CURRENT_SOURCE_DIR} -B ${CMAKE_CURRENT_BINARY_DIR}
			--graphviz="build_graph/${PROJECT_NAME}_graph.dot"
		COMMAND ${GRAPHVIZ_DOT_EXECUTABLE} -Tpng -o "build_graph/graph_of_build.png" "build_graph/${PROJECT_NAME}_graph.dot"
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()


# =========== INSTALLING AND PACKAGING ===========

option(INSTALL_AND_PACKAGE OFF)

if (INSTALL_AND_PACKAGE)
	install(TARGETS RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	if (CMAKE_SYSTEM_NAME STREQUAL Windows)
		install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/$<CONFIG>/"
			DESTINATION ${CMAKE_INSTALL_BINDIR} FILES_MATCHING PATTERN "*.dll")
	endif()

	include(InstallRequiredSystemLibraries)
	set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
	set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
	set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
	include(CPack)

	install(TARGETS DictionaryCreator_compiler_flags DictionaryCreatorConsoleApp
		EXPORT DictionaryCreatorTargets
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(EXPORT DictionaryCreatorTargets FILE DictionaryCreatorTargets.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/DictionaryCreator/cmake")

	include(CMakePackageConfigHelpers)
	configure_package_config_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/DictionaryCreatorConfig.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/DictionaryCreatorConfig.cmake"
		INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/DictionaryCreator/cmake"
		)
	write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/DictionaryCreatorConfigVersion.cmake"
		VERSION ${PROJECT_VERSION}
		COMPATIBILITY SameMajorVersion)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/DictionaryCreatorConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/DictionaryCreatorConfigVersion.cmake"
		DESTINATION "${CMAKE_INSTALL_LIBDIR}/DictionaryCreator/cmake")
endif ()
