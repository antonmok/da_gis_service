cmake_minimum_required(VERSION 3.1)

project(niokr_test)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../binaries/)

set(CMAKE_CXX_STANDARD 17)

set(CPP_FILES
    main.cpp)

add_executable(${PROJECT_NAME} ${CPP_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

