cmake_minimum_required(VERSION "3.12.0")

project("DictionaryCreator")
message("Project name is ${PROJECT_NAME}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE On)

include(FetchContent)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent.git
  GIT_TAG v3.7.3)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/whoshuu/cpr.git GIT_TAG c8d33915dbd88ad6c92b258869b03aba06587ff9) # the commit hash for 1.5.0
FetchContent_MakeAvailable(cpr)


add_executable(${PROJECT_NAME} "main.cpp")

add_library(utils STATIC "utils.cpp" "utils.h")
target_link_libraries(utils PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PUBLIC utils)

add_library(dictionary_creator STATIC "dictionary_creator.cpp" "dictionary_creator.h")

add_library(connections STATIC "connections.cpp" "connections.h")
target_link_libraries(connections PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(connections PUBLIC cpr::cpr)

target_link_libraries(dictionary_creator PUBLIC connections)

target_link_libraries(dictionary_creator PUBLIC utils)

target_link_libraries("${PROJECT_NAME}" PUBLIC dictionary_creator)




#[[
target_link_libraries(connections PUBLIC portability_utils)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent.git
  GIT_TAG v3.7.3)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_library(portability_utils "portability_utils.cpp" "portability_utils.h")
target_link_libraries(${PROJECT_NAME} PUBLIC portability_utils)

add_library(dictionary_creator "dictionary_creator.cpp" "dictionary_creator.h")
target_link_libraries(dictionary_creator PUBLIC portability_utils)
target_link_libraries(dictionary_creator PUBLIC nlohmann_json::nlohmann_json)

target_link_libraries("${PROJECT_NAME}" PUBLIC dictionary_creator)


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

# vcpkg says: CMake projects should use: "-DCMAKE_TOOLCHAIN_FILE=C:/src/vcpkg/scripts/buildsystems/vcpkg.cmake"
# about cpr it says:    
#						find_package(cpr CONFIG REQUIRED)
#					    target_link_libraries(main PRIVATE cpr)

# cpr requires OpenSSL but can't see versions newer than 1.0.2 in Windows enviroment
# vcpkg says following regarding openssl  
#set(OPENSSL_ROOT_DIR C:/src/vcpkg/installed/x86-windows)
#set(OPENSSL_ROOT_DIR C:/Program Files/OpenSSL)

#											    find_package(OpenSSL REQUIRED)
#											    target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
												
#set(CMAKE_TOOLCHAIN_FILE "C:/src/vcpkg/scripts/buildsystems/vcpkg.cmake")
# this one's not much of help
]]
