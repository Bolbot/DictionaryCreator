cmake_minimum_required(VERSION "3.15")

message("Library CMAKE_SOURCE_DIR:\t\t${CMAKE_SOURCE_DIR}")
message("Library CMAKE_CURRENT_SOURCE_DIR:\t${CMAKE_CURRENT_SOURCE_DIR}")
message("Library CMAKE_BINARY_DIR:\t\t${CMAKE_BINARY_DIR}")
message("Library CMAKE_CURRENT_BINARY_DIR:\t${CMAKE_CURRENT_BINARY_DIR}")

include (CheckIncludeFiles)

check_include_files(Windows.h WINDOWS_H_IS_AVAILABLE)
if (WINDOWS_H_IS_AVAILABLE)
	message("WINDOWS_H_IS_AVAILABLE")
endif (WINDOWS_H_IS_AVAILABLE)

check_include_files(io.h IO_H_IS_AVAILABLE)
if (IO_H_IS_AVAILABLE)
	message("IO_H_IS_AVAILABLE")
endif (IO_H_IS_AVAILABLE)

include(FetchContent)
FetchContent_Declare(json GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent.git GIT_TAG v3.7.3)
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
	FetchContent_Populate(json)
	add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


find_package(CURL REQUIRED)
if (CURL_FOUND)
	message("Using curl for connections:")
	message("CURL_INLUCDE_DIRS: ${CURL_INCLUDE_DIRS}")
	message("CURL_LIBRARIES: ${CURL_LIBRARIES}")
	message("CURL_VERSION: ${CURL_VERSION_STRING}")
else (CURL_FOUND)
	message("Curl could not be found")
endif (CURL_FOUND)


find_package(Boost REQUIRED COMPONENTS serialization)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(connections STATIC "connections.h" "connections.cpp")
target_link_libraries(connections PUBLIC CURL::libcurl)

add_library(dictionary_entry STATIC "dictionary_entry.h" "dictionary_entry.cpp" "dictionary_types.h")
target_link_libraries(dictionary_entry PUBLIC Boost::serialization)

add_library(dictionary_definer STATIC "dictionary_definer.h" "dictionary_definer.cpp" "dictionary_types.h" "dictionary_language.h")
target_link_libraries(dictionary_definer PUBLIC nlohmann_json::nlohmann_json connections Boost::serialization)

add_library(dictionary STATIC "dictionary.h" "dictionary.cpp" "dictionary_entry.h" "dictionary_types.h" "dictionary_language.h" "dictionary_definer.h")
target_link_libraries(dictionary PUBLIC dictionary_entry Boost::serialization)

add_library(regex_parser STATIC "regex_parser.h" "regex_parser.cpp")
find_package(unofficial-pcre CONFIG REQUIRED)
if (unofficial-pcre_FOUND)
	message("Found and linking unofficial-pcre")
	target_link_libraries(regex_parser PUBLIC unofficial::pcre::pcre)
else (unofficial-pcre_FOUND)
	message("unofficial-pcre wasn't found - ERROR")
endif (unofficial-pcre_FOUND)

add_library(dictionary_creator STATIC "dictionary_creator.h" "dictionary_creator.cpp" "regex_parser.h" "dictionary.h" "dictionary_definer.h")
target_link_libraries(dictionary_creator PUBLIC regex_parser dictionary)

add_library(dictionary_manager STATIC "dictionary_manager.h" "dictionary_manager.cpp" "dictionary.h" "dictionary_creator.h" "dictionary_definer.h")
target_link_libraries(dictionary_manager PUBLIC dictionary dictionary_creator dictionary_definer)
