cmake_minimum_required(VERSION 3.1)

project(websocket_srv)

set(CMAKE_INSTALL_PREFIX ${CMAKE_HOME_DIRECTORY}/binaries/)

set(CMAKE_CXX_STANDARD 17)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#set(H_CONFIG_FILES )

set(CPP_FILES
    webserver.cpp)

add_executable(${PROJECT_NAME} ${CPP_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

