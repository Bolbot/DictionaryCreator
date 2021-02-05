cmake_minimum_required(VERSION "3.12.0")

project("DictionaryCreator")
message("Project name is ${PROJECT_NAME}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE On)


if (UNIX)
# TODO replace this unreliable if (UNIX) with some CMakeCheckingPlatform recommended things
message("This is UNIX")
# just set the variables
endif (UNIX)

if (WIN32)
message("This is Windows")
set(CURL_LIBRARY "C:/src/vcpkg/packages/curl_x86-windows/lib")
set(CURL_INCLUDE_DIR "C:/src/vcpkg/packages/curl_x86-windows/include")
endif (WIN32)


include(FetchContent)

# JSON FEATURE
FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent.git
  GIT_TAG v3.7.3)
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(${PROJECT_NAME} "main.cpp")

add_library(utils STATIC "utils.cpp" "utils.h")
target_link_libraries(utils PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PUBLIC utils)

add_library(console_fs STATIC "console_file_system.cpp" "console_file_system.h")
target_link_libraries(console_fs PUBLIC utils)

add_library(dictionary_creator STATIC "dictionary_creator.cpp" "dictionary_creator.h")

add_library(connections STATIC "connections.cpp" "connections.h")

find_package(CURL)
if (CURL_FOUND)
message("Curl found, using curl for connections")
if (UNIX)
target_link_libraries(connections PUBLIC CURL::libcurl)	# is this thing redundant and causing all the troubles? looks like it's necessary for unix though
endif (UNIX)
else (CURL_FOUND)
message("Curl could not be found")
endif (CURL_FOUND)


target_link_libraries(connections PUBLIC utils)


add_library(regex_parser STATIC "regex_parser.cpp" "regex_parser.h")
if (UNIX)
target_link_libraries(regex_parser PUBLIC pcre)
endif (UNIX)

target_link_libraries(dictionary_creator PUBLIC connections regex_parser)

#target_link_libraries(dictionary_creator PUBLIC utils)
#redundant since utils is connected to connections already

target_link_libraries("${PROJECT_NAME}" PUBLIC dictionary_creator console_fs)


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
