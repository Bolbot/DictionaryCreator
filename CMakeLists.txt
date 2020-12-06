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

include(CPack)
# NSIS compiler failed to succeed because of some line 639 in project.nsi containing following...
# File /r "${INST_DIR}\*.*"
# this is the line mentioned as Error in script in NSISOutput.log

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/whoshuu/cpr.git GIT_TAG c8d33915dbd88ad6c92b258869b03aba06587ff9) # the commit hash for 1.5.0
FetchContent_MakeAvailable(cpr)

target_link_libraries("${PROJECT_NAME}" PRIVATE cpr::cpr)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent.git
  GIT_TAG v3.7.3)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

target_link_libraries("${PROJECT_NAME}" PRIVATE nlohmann_json::nlohmann_json)
