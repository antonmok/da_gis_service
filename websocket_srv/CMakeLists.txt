cmake_minimum_required(VERSION 3.1)

project(websocket_srv)

set(CMAKE_INSTALL_PREFIX ${CMAKE_HOME_DIRECTORY}/binaries/)

set(CMAKE_CXX_STANDARD 17)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package (glog 0.5.0 REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})

set(CPP_FILES
    protocol/socket_protocol.cpp
    ${CMAKE_HOME_DIRECTORY}/shared_image/shared_image.cpp
    ${CMAKE_HOME_DIRECTORY}/shared_image/shared_memory.cpp
    ${CMAKE_HOME_DIRECTORY}/shared_image/rwlock.cpp
    ${CMAKE_HOME_DIRECTORY}/base64.c)

set(CPP_TEST_FILES
    )

add_executable(${PROJECT_NAME} socket_server.cpp ${CPP_FILES})
#add_executable(${PROJECT_NAME}_test ${CPP_FILES} ${CPP_TEST_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS} Threads::Threads glog::glog rt jpeg)
#target_link_libraries(${PROJECT_NAME}_test PRIVATE Threads::Threads glog::glog rt jpeg)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
#add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
