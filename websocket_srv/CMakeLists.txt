cmake_minimum_required(VERSION 3.1)

project(websocket_srv)

set(CMAKE_INSTALL_PREFIX ${CMAKE_HOME_DIRECTORY}/binaries/)

set(CMAKE_CXX_STANDARD 17)

#set(H_CONFIG_FILES )

set(CPP_FILES
    webserver.cpp)

add_executable(${PROJECT_NAME} ${CPP_FILES})

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

