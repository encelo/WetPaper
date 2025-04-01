set(NCPROJECT_NAME "WetPaper")
set(NCPROJECT_EXE_NAME "wet_paper")
set(NCPROJECT_VENDOR "Team 3:00 a.m.")
set(NCPROJECT_COPYRIGHT "Copyright ©2025 ${NCPROJECT_VENDOR}")
set(NCPROJECT_DESCRIPTION "A game initially created for the Global Game Jam 2025")
set(NCPROJECT_HOMEPAGE "https://encelo.itch.io/wet_paper")
set(NCPROJECT_REVERSE_DNS "io.itch.encelo.wet_paper")

set(NCPROJECT_SOURCES
	src/main.h
	src/main.cpp
	src/Config.h
	src/Settings.h
	src/Statistics.h
	src/DebugDraw.h
	src/nodes/Body.h
	src/nodes/Body.cpp
	src/nodes/Game.h
	src/nodes/Game.cpp
	src/nodes/LogicNode.h
	src/nodes/LogicNode.cpp
	src/nodes/Player.h
	src/nodes/Player.cpp
	src/nodes/Bubble.h
	src/nodes/Bubble.cpp
	src/nodes/SplashScreen.h
	src/nodes/SplashScreen.cpp
	src/nodes/Menu.h
	src/nodes/Menu.cpp
	src/nodes/MenuPage.h
	src/nodes/MenuPage.cpp
	src/ResourceManager.h
	src/ResourceManager.cpp
	src/InputBinder.h
	src/InputBinder.cpp
	src/InputActions.h
	src/InputActions.cpp
	src/InputNames.h
	src/InputNames.cpp
)

option(CUSTOM_ITCHIO_BUILD "Create a build for the Itch.io store" ON)

function(callback_start)
	if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		set(NCPROJECT_ALWAYS_FIND_PACKAGES ${CUSTOM_ITCHIO_BUILD} CACHE BOOL "Always find packages for Itch.io builds" FORCE)
	endif()
endfunction()

function(callback_before_target)
	if(CUSTOM_ITCHIO_BUILD)
		if(NOT APPLE)
			install(FILES .itch.toml DESTINATION .)
		endif()
		if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
			install(FILES launch.sh
				PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
				DESTINATION .)

			install_linux_library(ncine::ncine)
			if(NCINE_WITH_GLEW)
				install_linux_library(GLEW::GLEW)
			endif()
			if(NCINE_WITH_SDL)
				install_linux_library(SDL2::SDL2)
			elseif(NCINE_WITH_GLFW)
				install_linux_library(GLFW::GLFW)
			endif()
			if(NCINE_WITH_PNG)
				install_linux_library(ZLIB::ZLIB)
				install_linux_library(PNG::PNG)
			endif()
			if(NCINE_WITH_AUDIO)
				install_linux_library(OpenAL::AL)
				if(NCINE_WITH_VORBIS)
					install_linux_library(Ogg::Ogg)
					install_linux_library(Vorbis::Vorbis)
					install_linux_library(Vorbis::Vorbisfile)
				endif()
			endif()
		endif()
	endif()
endfunction()

function(callback_end)
	if(CUSTOM_ITCHIO_BUILD AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NCPROJECT_OPTIONS_PRESETS STREQUAL "BinDist")
		# Set a relative data directory when building for Itch.io on Linux
		file(RELATIVE_PATH NCPROJECT_DEFAULT_DATA_DIR_NEW
				${CMAKE_INSTALL_PREFIX}/${RUNTIME_INSTALL_DESTINATION}
				${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DESTINATION}) # Always strips trailing slash
		set(NCPROJECT_DEFAULT_DATA_DIR_NEW "${NCPROJECT_DEFAULT_DATA_DIR_NEW}/")

		get_target_property(COMPILE_DEFS ${NCPROJECT_EXE_NAME} COMPILE_DEFINITIONS)
		string(REPLACE ${NCPROJECT_DEFAULT_DATA_DIR} ${NCPROJECT_DEFAULT_DATA_DIR_NEW} COMPILE_DEFS "${COMPILE_DEFS}")
		set_property(TARGET ${NCPROJECT_EXE_NAME} PROPERTY COMPILE_DEFINITIONS ${COMPILE_DEFS})
	endif()
endfunction()

function(install_linux_library TARGET_NAME)
	get_target_property(LIB_LOCATION ${TARGET_NAME} IMPORTED_LOCATION)
	if(NOT LIB_LOCATION)
		get_target_property(LIB_LOCATION ${TARGET_NAME} IMPORTED_LOCATION_RELEASE)
	endif()
	get_filename_component(LIB_LOCATION ${LIB_LOCATION} REALPATH) # resolve symlink
	install(FILES ${LIB_LOCATION} DESTINATION lib64)

	get_filename_component(LIB_NAME ${LIB_LOCATION} NAME)
	set(LIB_NAME_STRIPPED ${LIB_NAME})
	string(REGEX MATCH "\\.[0-9]+$" LIB_REGEX_MATCH ${LIB_NAME_STRIPPED})
	while(LIB_REGEX_MATCH)
		string(REGEX REPLACE "\\.[0-9]+$" "" LIB_NAME_STRIPPED ${LIB_NAME_STRIPPED}) # remove last version number
		install(CODE "file(CREATE_LINK \"${LIB_NAME}\" \"\${CMAKE_INSTALL_PREFIX}/lib64/${LIB_NAME_STRIPPED}\" SYMBOLIC)")
		string(REGEX MATCH "\\.[0-9]+$" LIB_REGEX_MATCH ${LIB_NAME_STRIPPED})
	endwhile()
endfunction()

# Don't edit beyond this line
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
	return()
endif()

cmake_minimum_required(VERSION 3.13)
project(${NCPROJECT_NAME})

find_path(NCPROJECT_DIR cmake/project_main.cmake
	PATHS ${nCine_DIR} $ENV{NCINE_DIR}
	PATH_SUFFIXES project ../project ../../../project ../../../share/ncine/project
	NO_CMAKE_FIND_ROOT_PATH # For Emscripten
	DOC "Path to the nCine template project scripts")

if(IS_DIRECTORY ${NCPROJECT_DIR})
	include(${NCPROJECT_DIR}/cmake/project_main.cmake)
else()
	if(NOT DEFINED nCine_DIR AND "$ENV{NCINE_DIR}" STREQUAL "")
		message(FATAL_ERROR "Set the nCine_DIR CMake variable or the NCINE_DIR environment variable to a valid nCine directory.")
	elseif(DEFINED nCine_DIR)
		message(FATAL_ERROR "The nCine_DIR CMake variable \"${nCine_DIR}\" is not a valid nCine directory.")
	elseif(NOT "$ENV{NCINE_DIR}" STREQUAL "")
		message(FATAL_ERROR "The NCINE_DIR environment variable \"$ENV{NCINE_DIR}\" is not a valid nCine directory.")
	endif()
endif()
