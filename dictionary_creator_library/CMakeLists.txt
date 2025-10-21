# ========== THIRD-PARTY ==========

find_package(CURL CONFIG QUIET)                         # for connections library
find_package(pcre2 CONFIG QUIET)                        # for regex_parser library

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

set(Boost_NO_BOOST_CMAKE ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS serialization CONFIG QUIET)					# for almost every dictionary library


# ========== DECLARATIONS ==========

add_library(connections connections.cpp)
if (CURL_FOUND)
	target_link_libraries(connections PRIVATE CURL::libcurl)
	target_compile_definitions(connections PRIVATE "CURL_IS_AVALIABLE")
else ()
	# connections expects CURL_IS_AVALIABLE defined from outside; by skipping it, we make it fallback to no-op
	message(WARNING "Failed to find Curl. The connections library will be no-op")
endif ()

add_library(regex_parser regex_parser.cpp)

if (pcre2_FOUND)
	message(WARNING "PCRE2 was found right away. Suspiciously successful")
else ()
	FetchContent_Declare(pcre URL https://github.com/PCRE2Project/pcre2/releases/download/pcre2-10.46/pcre2-10.46.tar.gz)
	FetchContent_MakeAvailable(pcre)
endif()
target_link_libraries(regex_parser PRIVATE pcre2-8 DictionaryCreator_compiler_flags)
target_compile_definitions(regex_parser PRIVATE "PCRE2_CODE_UNIT_WIDTH=8")


add_library(dictionary_definer dictionary_definer.cpp)
target_link_libraries(dictionary_definer PRIVATE nlohmann_json::nlohmann_json connections DictionaryCreator_compiler_flags)

add_library(dictionary_entry dictionary_entry.cpp)
target_link_libraries(dictionary_entry PUBLIC PRIVATE DictionaryCreator_compiler_flags)

add_library(dictionary dictionary.cpp)
target_link_libraries(dictionary PUBLIC dictionary_entry PRIVATE DictionaryCreator_compiler_flags)

add_library(dictionary_creator dictionary_creator.cpp)
target_link_libraries(dictionary_creator PUBLIC dictionary regex_parser PRIVATE DictionaryCreator_compiler_flags)

add_library(dictionary_manager dictionary_manager.cpp)
target_link_libraries(dictionary_manager PUBLIC dictionary_creator PRIVATE dictionary_definer DictionaryCreator_compiler_flags)
target_include_directories(dictionary_manager PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
add_library(DictionaryCreator ALIAS dictionary_manager)

set_target_properties(dictionary_manager dictionary_creator dictionary dictionary_entry dictionary_definer 
	PROPERTIES FOLDER dictionary_creator)

option (SERIALIZE OFF)
if (Boost_FOUND AND SERIALIZE)
	target_link_libraries(dictionary_definer PRIVATE Boost::serialization)
	target_link_libraries(dictionary_entry   PRIVATE Boost::serialization)
	target_link_libraries(dictionary         PRIVATE Boost::serialization)
	target_link_libraries(dictionary_creator PRIVATE Boost::serialization)
	target_link_libraries(dictionary_manager PRIVATE Boost::serialization)
else ()
	message(WARNING "Serialization will be no-op (Found Boost ${Boost_FOUND}\tSERIALIZE option ${SERIALIZE})")
	target_compile_definitions(dictionary_definer PRIVATE "BOOST_UNAVAILABLE")
	target_compile_definitions(dictionary_entry   PRIVATE "BOOST_UNAVAILABLE")
	target_compile_definitions(dictionary         PRIVATE "BOOST_UNAVAILABLE")
	target_compile_definitions(dictionary_creator PRIVATE "BOOST_UNAVAILABLE")
	target_compile_definitions(dictionary_manager PUBLIC  "BOOST_UNAVAILABLE")		# required by DictionaryCreatorConsoleApp
endif()

# ========== INSTALLATION ==========

install(FILES dictionary_manager.h dictionary_entry.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS dictionary_manager dictionary_definer dictionary_creator dictionary regex_parser dictionary_entry connections nlohmann_json
	EXPORT DictionaryCreatorTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
set_target_properties(dictionary_manager dictionary_definer dictionary_creator dictionary regex_parser dictionary_entry connections
	PROPERTIES
		INSTALL_RPATH $ORIGIN
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR})
