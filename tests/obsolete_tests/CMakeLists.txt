add_executable(simple_test_obsolete simple_tests.h simple_tests.cpp)
add_executable(huge_test_obsolete huge_test.h huge_test.cpp)
add_executable(boost_test boost_tests.cpp)

target_link_libraries(simple_test_obsolete PUBLIC dictionary_manager)

target_link_libraries(boost_test PUBLIC ${Boost_LIBRARIES} dictionary_entry connections dictionary_definer dictionary dictionary_creator)
