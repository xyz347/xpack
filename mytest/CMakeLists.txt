
cmake_minimum_required(VERSION 3.0)

project("xpack test" C CXX)

option(BUILD_1_FIRST_TEST  "Build 1-first program"      OFF)
option(BUILD_2_SECOND_TEST "Build 2-second program"     OFF)

#
# if u wanna debug, do like this `cmake -DCMAKE_BUILD_TYPE=Debug`
#
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include(CheckCXXCompilerFlag)

#
# Configuration
#
include("${PROJECT_SOURCE_DIR}/cmake/configuration.cmake")

#
# Dependencies
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

#
# Package - rapidjson
#
find_package(rapidjson REQUIRED)
include_directories(SYSTEM ${rapidjson_INCLUDE})

#
# Test in Sub Directories
#
if(BUILD_1_FIRST_TEST)
    add_subdirectory(1-first)
endif()

if(BUILD_2_SECOND_TEST)
    add_subdirectory(2-second)
endif()

#
# End
#
