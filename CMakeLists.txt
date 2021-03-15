cmake_minimum_required(VERSION 3.1)

project(blabber)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/binaries/)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package (glog 0.5.0 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR})

message(STATUS "${CMAKE_SYSTEM_PROCESSOR}")

#set(H_CONFIG_FILES )

set(CPP_FILES
    base64.c
    shared_image/shared_image.cpp
    shared_image/shared_memory.cpp
    shared_image/rwlock.cpp)

set(CPP_TEST_FILES
    )

enable_testing()
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "X86_64")
    add_subdirectory(wxUI)
endif()

add_subdirectory(websocket_srv)
add_subdirectory(http_server)

add_executable(${PROJECT_NAME} main.cpp ${CPP_FILES})
#add_executable(${PROJECT_NAME}_test ${CPP_FILES} ${CPP_TEST_FILES})

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${OpenCV_LIBS} websockets stdc++fs pthread rt jpeg glog::glog)
#target_link_libraries(${PROJECT_NAME}_test PRIVATE ${CMAKE_THREAD_LIBS_INIT} ${OpenCV_LIBS} websockets stdc++fs pthread rt jpeg glog::glog)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
#add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh ${CMAKE_INSTALL_PREFIX}/run.sh COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/stop.sh ${CMAKE_INSTALL_PREFIX}/stop.sh COPYONLY)
