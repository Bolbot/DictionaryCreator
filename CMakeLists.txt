cmake_minimum_required(VERSION "3.12.0")

project("DictionaryCreator")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE On)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	add_definitions(-D_UNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


message("Current OS: ${CMAKE_SYSTEM_NAME} (version ${CMAKE_SYSTEM_VERSION})")
message("Current processor: ${CMAKE_SYSTEM_PROCESSOR}")
if (${CMAKE_TOOLCHAIN_FILE})
	message("Using toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif (${CMAKE_TOOLCHAIN_FILE})


include (CheckIncludeFiles)

check_include_files(Windows.h WINDOWS_H_IS_AVAILABLE)
if (WINDOWS_H_IS_AVAILABLE)
	message("WINDOWS_H_IS_AVAILABLE")
endif (WINDOWS_H_IS_AVAILABLE)

check_include_files(io.h IO_H_IS_AVAILABLE)
if (IO_H_IS_AVAILABLE)
	message("IO_H_IS_AVAILABLE")
endif (IO_H_IS_AVAILABLE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
message("Check ${CMAKE_CURRENT_SOURCE_DIR}/config.h made from ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in")
#works like this, only takes rebuilds with _H variables deleted from CMakeCache


include(FetchContent)
FetchContent_Declare(json GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent.git GIT_TAG v3.7.3)
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
	FetchContent_Populate(json)
	add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


find_package(CURL)
if (CURL_FOUND)
	message("Using curl for connections:")
	message("CURL_INLUCDE_DIRS: ${CURL_INCLUDE_DIRS}")
	message("CURL_LIBRARIES: ${CURL_LIBRARIES}")
	message("CURL_VERSION: ${CURL_VERSION_STRING}")
else (CURL_FOUND)
	message("Curl could not be found")
endif (CURL_FOUND)


find_package(Boost REQUIRED serialization)


add_executable(${PROJECT_NAME} "main.cpp" "simple_tests.h")


add_library(connections STATIC "connections.h" "connections.cpp")
target_link_libraries(connections PUBLIC CURL::libcurl)

add_library(dictionary_entry STATIC "dictionary_entry.h" "dictionary_entry.cpp" "dictionary_types.h")
target_link_libraries(dictionary_entry PUBLIC Boost::serialization)

add_library(dictionary_definer STATIC "dictionary_definer.h" "dictionary_definer.cpp" "dictionary_types.h" "dictionary_language.h")
target_link_libraries(dictionary_definer PUBLIC nlohmann_json::nlohmann_json connections Boost::serialization)

add_library(dictionary STATIC "dictionary.h" "dictionary.cpp" "dictionary_entry.h" "dictionary_types.h" "dictionary_language.h" "dictionary_definer.h")
target_link_libraries(dictionary PUBLIC dictionary_entry Boost::serialization)

add_library(regex_parser STATIC "regex_parser.h" "regex_parser.cpp")
check_include_files(pcre.h PCRE_H_IS_AVAILABLE)
if (PCRE_H_IS_AVAILABLE)
	target_link_libraries(regex_parser PUBLIC pcre)
else (PCRE_H_IS_AVALIABLE)
	find_package(unofficial-pcre CONFIG REQUIRED)
	target_link_libraries(regex_parser PUBLIC unofficial::pcre::pcre)
endif (PCRE_H_IS_AVAILABLE)

add_library(dictionary_creator STATIC "dictionary_creator.h" "dictionary_creator.cpp" "regex_parser.h" "dictionary.h" "dictionary_definer.h")
target_link_libraries(dictionary_creator PUBLIC regex_parser dictionary)

add_library(dictionary_manager STATIC "dictionary_manager.h" "dictionary_manager.cpp" "dictionary.h" "dictionary_creator.h" "dictionary_definer.h")
target_link_libraries(dictionary_manager PUBLIC dictionary dictionary_creator dictionary_definer)

add_library(simple_tests STATIC "simple_tests.h" "simple_tests.cpp")
target_link_libraries(simple_tests PUBLIC connections dictionary_entry dictionary regex_parser dictionary_creator dictionary_manager)

add_library(huge_test STATIC "huge_test.h" "huge_test.cpp")
target_link_libraries(huge_test PUBLIC dictionary_manager)

target_link_libraries(${PROJECT_NAME} PUBLIC simple_tests huge_test)

# additional features for further use

#[[
install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
message("Is going to install to ${CMAKE_INSTALL_PREFIX}/bin that is CMAKE_INSTALL_PREFIX/bin")

install(FILES "main.cpp" DESTINATION src)
message("Is going to install main.cpp to ${CMAKE_INSTALL_PREFIX}/src that is once again CMAKE_INSTALL_PREFIX/src")

include(CTest)
message("Executable is going to be created in ${CMAKE_CURRENT_BINARY_DIR} that is CMAKE_CURRENT_BINARY_DIR")
add_test("Arguments_absence_test_pass" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}")

include(CPack)
# NSIS compiler failed to succeed because of some line 639 in project.nsi containing following...
# File /r "${INST_DIR}\*.*"
# this is the line mentioned as Error in script in NSISOutput.log
]]
