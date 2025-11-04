message(STATUS "Building tests for ${PROJECT_NAME}")

if (${BUILD_SHARED_LIBS})
	set(Boost_USE_STATIC_LIBS OFF)
	set(Boost_USE_STATIC_RUNTIME OFF)
else()
	set(Boost_USE_STATIC_LIBS ON)
	set(Boost_USE_STATIC_RUNTIME ON)
endif()
set(Boost_USE_MULTITHREAD ON)
set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

function(add_boost_test FEATURENAME)
	set(test_name boost_test_${FEATURENAME})
	set(files_location "${CMAKE_SOURCE_DIR}/dictionary_creator_library/")
	list(APPEND ARGN ${FEATURENAME})
	set(dependencies ${ARGN})
	set(headers ${ARGN})
	list(TRANSFORM headers PREPEND ${files_location})
	list(TRANSFORM headers APPEND ".h")

	add_executable(${test_name} ${test_name}.cpp ${headers})
	target_include_directories(${test_name} PRIVATE ${files_location})
	target_link_libraries(${test_name} PRIVATE ${dependencies} Boost::unit_test_framework ${PROJECT_NAME}_compiler_flags)
	target_compile_definitions(${test_name} PRIVATE $<$<BOOL:${BUILD_SHARED_LIBS}>:BOOST_TEST_DYN_LINK>)

	add_test(NAME ${test_name} COMMAND ${test_name})
	set_tests_properties(${test_name} PROPERTIES PASS_REGULAR_EXPRESSION "No errors detected")
	set_property(TARGET ${test_name} PROPERTY FOLDER "tests/boost")
endfunction()

# aux libraries, external to dictionary creator
add_boost_test(regex_parser)
add_boost_test(json_parser)
find_package(nlohmann_json)                  # for json_parser
target_link_libraries(boost_test_json_parser PRIVATE nlohmann_json::nlohmann_json)
add_boost_test(connections)

# core dictionary classes
add_boost_test(dictionary_entry)
add_boost_test(dictionary_definer)
add_boost_test(dictionary dictionary_exporter dictionary_definer)

# auxiliary classes
add_boost_test(dictionary_exporter dictionary)
add_boost_test(dictionary_creator)

if (SERIALIZATION)
	add_boost_test(dictionary_manager)
endif()

add_subdirectory(obsolete_tests)
