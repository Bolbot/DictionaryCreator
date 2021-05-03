cmake_minimum_required(VERSION 3.15)


message("Tests CMAKE_SOURCE_DIR:\t\t${CMAKE_SOURCE_DIR}")
message("Tests CMAKE_CURRENT_SOURCE_DIR:\t${CMAKE_CURRENT_SOURCE_DIR}")
message("Tests CMAKE_BINARY_DIR:\t\t${CMAKE_BINARY_DIR}")
message("Tests CMAKE_CURRENT_BINARY_DIR:\t${CMAKE_CURRENT_BINARY_DIR}")

set(Boost_USE_STATIC_LIBS OFF)

find_package(Boost REQUIRED COMPONENTS unit_test_framework serialization)

include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(tests)

add_executable(simple_test_obsolete simple_tests.h simple_tests.cpp)
#add_executable(huge_test_obsolete huge_test.h huge_test.cpp)
add_executable(boost_test boost_tests.cpp)

target_link_libraries(simple_test_obsolete PUBLIC dictionary_manager)

target_link_libraries(boost_test PUBLIC ${Boost_LIBRARIES} dictionary_entry connections dictionary_definer dictionary dictionary_creator)
