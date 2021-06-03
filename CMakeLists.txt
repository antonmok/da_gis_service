cmake_minimum_required(VERSION 3.1)

project(blabber)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/binaries/)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

message(STATUS "${CMAKE_SYSTEM_PROCESSOR}")

#set(H_CONFIG_FILES )

set(CPP_FILES main.cpp base64.c)

enable_testing()
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "X86_64")
    add_subdirectory(wxUI)
endif()

add_subdirectory(websocket_srv)
add_subdirectory(http_server)

add_executable(${PROJECT_NAME} ${CPP_FILES})
target_link_libraries( ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${OpenCV_LIBS} websockets stdc++fs pthread rt jpeg)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh ${CMAKE_INSTALL_PREFIX}/run.sh COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/stop.sh ${CMAKE_INSTALL_PREFIX}/stop.sh COPYONLY)
