if (UNIX)
	set(OS_TYPE unix)
elseif(WIN32)
	set(OS_TYPE windows)
else()
	message(FATAL_ERROR "Unknown OS. Impossible to continue (for application build)")
endif()

add_library(console_fs_manager "${OS_TYPE}/fs_manager.cpp")
target_include_directories(console_fs_manager PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${OS_TYPE}")
target_link_libraries(console_fs_manager PRIVATE ${PROJECT_NAME}_compiler_flags)
set_target_properties(console_fs_manager PROPERTIES
	VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

if (${CMAKE_SYSTEM_NAME} STREQUAL Windows AND ${BUILD_SHARED_LIBS})
	target_compile_definitions(console_fs_manager PRIVATE "OPTIONAL_DECLSPEC=__declspec(dllexport)" INTERFACE "OPTIONAL_DECLSPEC=__declspec(dllimport)")
endif()

if (${CMAKE_GENERATOR} MATCHES "Visual Studio")
	target_compile_options(console_fs_manager PUBLIC "/Zc:externConstexpr" "/Zc:__cplusplus")
endif()

install(TARGETS console_fs_manager
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
