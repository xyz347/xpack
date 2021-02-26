
cmake_minimum_required(VERSION 3.0)

project("xpack test" C CXX)

option(BUILD_01_first   "Build 01_first program"       ON)
option(BUILD_02_Second  "Build 02_second program"      ON)

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

include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/../../)

#
# Enable test
#
enable_testing()

#
# Test Suites
#
if(BUILD_01_first)
    add_subdirectory(01_first)
endif()

if(BUILD_02_Second)
    add_subdirectory(02_second)
endif()


#
# End
#
