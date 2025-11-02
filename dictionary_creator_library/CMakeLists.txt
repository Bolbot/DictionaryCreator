# ========== THIRD-PARTY ==========

find_package(CURL REQUIRED)                         # for connections library
find_package(pcre2 REQUIRED)                        # for regex_parser library
find_package(nlohmann_json)                         # for dictionary_definer library

set(Boost_NO_BOOST_CMAKE ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS serialization CONFIG QUIET)  # SERIALIZATION is optional and off by default


# ========== DECLARATIONS ==========

add_library(connections connections.cpp connections.h)
if (CURL_FOUND)
	target_link_libraries(connections PRIVATE CURL::libcurl)
	target_compile_definitions(connections PRIVATE "CURL_IS_AVALIABLE")
else ()
	# connections expects CURL_IS_AVALIABLE defined from outside; by skipping it, we make it fallback to no-op
	message(WARNING "Failed to find Curl. The connections library will be no-op")
endif ()

add_library(regex_parser regex_parser.cpp regex_parser.h)
target_link_libraries(regex_parser PRIVATE PCRE2::8BIT DictionaryCreator_compiler_flags)
target_compile_definitions(regex_parser PRIVATE "PCRE2_CODE_UNIT_WIDTH=8")

add_library(json_parser json_parser.cpp json_parser.h json_parser_impl.h)
target_link_libraries(json_parser PRIVATE nlohmann_json::nlohmann_json DictionaryCreator_compiler_flags)
target_compile_definitions(json_parser PRIVATE "BOOST_UNAVAILABLE")   # json parser doesn't need boost serialization types

add_library(dictionary_definer dictionary_definer.cpp dictionary_definer.h dictionary_types.h dictionary_language.h)
target_link_libraries(dictionary_definer PRIVATE json_parser connections DictionaryCreator_compiler_flags)

add_library(dictionary_entry dictionary_entry.cpp dictionary_entry.h dictionary_types.h)
target_link_libraries(dictionary_entry PUBLIC PRIVATE DictionaryCreator_compiler_flags)

add_library(dictionary dictionary.cpp dictionary.h dictionary_types.h dictionary_entry.h dictionary_language.h)
target_link_libraries(dictionary PUBLIC dictionary_entry PRIVATE DictionaryCreator_compiler_flags)

add_library(dictionary_creator dictionary_creator.cpp dictionary_creator.h regex_parser.h dictionary.h)
target_link_libraries(dictionary_creator PUBLIC dictionary regex_parser PRIVATE DictionaryCreator_compiler_flags)

add_library(dictionary_exporter dictionary_exporter.cpp dictionary_exporter.h dictionary.h dictionary_types.h)
target_link_libraries(dictionary_exporter PRIVATE DictionaryCreator_compiler_flags)
target_compile_definitions(dictionary_exporter PRIVATE "BOOST_UNAVAILABLE")

add_library(dictionary_manager dictionary_manager.cpp dictionary_manager.h dictionary.h dictionary_creator.h)
target_link_libraries(dictionary_manager PUBLIC dictionary_creator PRIVATE dictionary_definer dictionary_exporter DictionaryCreator_compiler_flags)
target_include_directories(dictionary_manager PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
add_library(DictionaryCreator ALIAS dictionary_manager)

set_target_properties(dictionary_manager dictionary_creator dictionary dictionary_entry dictionary_definer dictionary_exporter
	PROPERTIES FOLDER dictionary_creator)


# ========== SERIALIZATION FEATURE ==========

option (SERIALIZATION OFF)
if (Boost_FOUND AND SERIALIZATION)
	target_link_libraries(dictionary_definer PRIVATE Boost::serialization)
	target_link_libraries(dictionary_entry   PRIVATE Boost::serialization)
	target_link_libraries(dictionary         PRIVATE Boost::serialization)
	target_link_libraries(dictionary_creator PRIVATE Boost::serialization)
	target_link_libraries(dictionary_manager PUBLIC Boost::serialization)               # required by DictionaryCreatorConsoleApp
else ()
	message(WARNING "Serialization will be no-op (Found Boost ${Boost_FOUND}\tSERIALIZE option ${SERIALIZE})")
	target_compile_definitions(dictionary_definer PRIVATE "BOOST_UNAVAILABLE")
	target_compile_definitions(dictionary_entry   PRIVATE "BOOST_UNAVAILABLE")
	target_compile_definitions(dictionary         PRIVATE "BOOST_UNAVAILABLE")
	target_compile_definitions(dictionary_creator PRIVATE "BOOST_UNAVAILABLE")
	target_compile_definitions(dictionary_manager PUBLIC  "BOOST_UNAVAILABLE")          # required by DictionaryCreatorConsoleApp
endif()


# ========== INSTALLATION ==========

if (INSTALL_AND_PACKAGE)
	install(FILES dictionary_manager.h dictionary_entry.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
	install(TARGETS dictionary_manager dictionary_definer dictionary_creator dictionary regex_parser dictionary_entry connections nlohmann_json::nlohmann_json
		EXPORT DictionaryCreatorTargets
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
	set_target_properties(dictionary_manager dictionary_definer dictionary_creator dictionary regex_parser dictionary_entry connections
		PROPERTIES
			INSTALL_RPATH $ORIGIN
			VERSION ${PROJECT_VERSION}
			SOVERSION ${PROJECT_VERSION_MAJOR})
endif()