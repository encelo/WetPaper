set(NCPROJECT_NAME "WetPaper")
set(NCPROJECT_EXE_NAME "wet_paper")
set(NCPROJECT_VENDOR "Team 3:00 a.m.")
set(NCPROJECT_COPYRIGHT "Copyright ©2025 ${NCPROJECT_VENDOR}")
set(NCPROJECT_DESCRIPTION "A game initially created for the Global Game Jam 2025")
set(NCPROJECT_HOMEPAGE "https://encelo.itch.io/wet_paper")
set(NCPROJECT_REVERSE_DNS "io.github.encelo.wet_paper")

set(NCPROJECT_SOURCES
	src/main.h
	src/main.cpp
	src/Config.h
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
	src/nodes/Menu.h
	src/nodes/Menu.cpp
	src/ResourceManager.h
	src/ResourceManager.cpp
)

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
