CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(sonar)

#
# compiler settings
#
SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Weffc++ -pedantic -pthread -O2")
SET(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")

#
# build script for OTF lib
#
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/libs/include/open-trace-format/otf.h)
	MESSAGE(STATUS "OTF library seems fine... skipping build-process")
else()
	MESSAGE(STATUS "Building OTF library")
	MESSAGE(STATUS ${CMAKE_CURRENT_LIST_DIR})
	EXECUTE_PROCESS(
		COMMAND "${CMAKE_CURRENT_LIST_DIR}/otfbuild.sh"
		WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")
endif()

FILE(GLOB SOURCES "src/*.cpp")
FILE(GLOB_RECURSE LibFiles "inc/*.h")          # for Qt Creator
add_custom_target(headers SOURCES ${LibFiles}) # for Qt Creator

INCLUDE_DIRECTORIES("libs/include/open-trace-format") # == -I/path/to/inc
INCLUDE_DIRECTORIES("inc")
cmake_policy(SET CMP0015 NEW)
LINK_DIRECTORIES("libs/lib") # == -L/path/to/lib

ADD_EXECUTABLE(sonar ${SOURCES})
TARGET_LINK_LIBRARIES(sonar open-trace-format ) # == -lmylib

#
# testing
#
add_test(NAME Valgrind WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND valgrind ./sonar ${CMAKE_CURRENT_LIST_DIR}/sampletraces/lulesh_8p.otf)

SET (CTEST_OUTPUT_ON_FAILURE)
enable_testing()
