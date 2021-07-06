# ========== THIRD-PARTY ==========

find_package(CURL REQUIRED)								# for connections library
find_package(unofficial-pcre REQUIRED)							# for regex_parser library

include(FetchContent)
FetchContent_Declare(json								# for dictionary_definer library
	GIT_REPOSITORY	git@github.com:Bolbot/nlohmann_json_cmake_fetchcontent.git
	GIT_TAG		e203a1f0e31674e11f41fe4afc964a20fd12c50d)
FetchContent_MakeAvailable(json)

set(Boost_NO_BOOST_CMAKE ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS serialization)					# for almost every dictionary library


# ========== DECLARATIONS ==========

add_library(connections connections.cpp)
target_link_libraries(connections PRIVATE CURL::libcurl)

add_library(regex_parser regex_parser.cpp)
target_link_libraries(regex_parser PRIVATE unofficial::pcre::pcre DictionaryCreator_compiler_flags)


add_library(dictionary_definer dictionary_definer.cpp)
target_link_libraries(dictionary_definer PRIVATE nlohmann_json::nlohmann_json connections Boost::serialization DictionaryCreator_compiler_flags)

add_library(dictionary_entry dictionary_entry.cpp)
target_link_libraries(dictionary_entry PUBLIC Boost::serialization PRIVATE DictionaryCreator_compiler_flags)

add_library(dictionary dictionary.cpp)
target_link_libraries(dictionary PUBLIC dictionary_entry PRIVATE Boost::serialization DictionaryCreator_compiler_flags)

add_library(dictionary_creator dictionary_creator.cpp)
target_link_libraries(dictionary_creator PUBLIC dictionary regex_parser PRIVATE DictionaryCreator_compiler_flags)

add_library(dictionary_manager dictionary_manager.cpp)
target_link_libraries(dictionary_manager PUBLIC dictionary_creator PRIVATE dictionary_definer Boost::serialization DictionaryCreator_compiler_flags)
target_include_directories(dictionary_manager PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
add_library(DictionaryCreator ALIAS dictionary_manager)

set_target_properties(dictionary_manager dictionary_creator dictionary dictionary_entry dictionary_definer 
	PROPERTIES FOLDER dictionary_creator)


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
