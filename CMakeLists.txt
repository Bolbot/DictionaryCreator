cmake_minimum_required(VERSION "3.12.0")

project("DictionaryCreator" VERSION 0.8.5 LANGUAGES CXX)

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
# BIG question is HOW to forbid CMAKE from caching this WINDOWS_H_IS_AVAILABLE variable, it's so annoying to delete it manually each time
if (WINDOWS_H_IS_AVAILABLE)
	message("WINDOWS_H_IS_AVAILABLE")
endif (WINDOWS_H_IS_AVAILABLE)

check_include_files(io.h IO_H_IS_AVAILABLE)
if (IO_H_IS_AVAILABLE)
	message("IO_H_IS_AVAILABLE")
endif (IO_H_IS_AVAILABLE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
message("Check ${CMAKE_CURRENT_SOURCE_DIR}/config.h made from ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in")


add_executable(${PROJECT_NAME} "main.cpp")

add_subdirectory(dictionary_creator_library)
include_directories(dictionary_creator_library)

add_subdirectory(tests)

target_link_libraries(${PROJECT_NAME} PUBLIC dictionary_manager)


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
