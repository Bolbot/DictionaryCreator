cmake_minimum_required(VERSION "3.12.0")

project("DictionaryCreator")
message("Project name is ${PROJECT_NAME}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable("${PROJECT_NAME}" "main.cpp")

install(TARGETS "${PROJECT_NAME}" DESTINATION bin)		# this line will force CMAKE to add this program to windows program files ${CMAKE_INSTALL_PREFIX}/bin
message("Is going to install to ${CMAKE_INSTALL_PREFIX}/bin that is CMAKE_INSTALL_PREFIX/bin")

install(FILES "main.cpp" DESTINATION src)				# this line guarantees that CMAKE..
message("Is going to install main.cpp to ${CMAKE_INSTALL_PREFIX}/src that is once again CMAKE_INSTALL_PREFIX/src")

include(CTest)

message("Executable is going to be created in ${CMAKE_CURRENT_BINARY_DIR} that is CMAKE_CURRENT_BINARY_DIR")

add_test("Arguments_absence_test_pass" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}")
