
cmake_minimum_required(VERSION 3.0)

file(COPY test.cad DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include_directories(.)


add_executable(cad2json cad2json.cpp)

add_executable(json2struct json2struct.cpp)

add_test(01_first_test_step_1 cad2json test.cad test.json)
add_test(01_first_test_step_2 json2struct test.json)