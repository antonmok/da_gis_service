cmake_minimum_required(VERSION 3.1)

project(unit_tests)

add_executable(${PROJECT_NAME}_api api.cpp)
add_executable(${PROJECT_NAME}_algo algo.cpp)

add_test(NAME api_tests COMMAND ${PROJECT_NAME}_api)
add_test(NAME algo_tests COMMAND ${PROJECT_NAME}_algo) 
