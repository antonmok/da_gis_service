cmake_minimum_required(VERSION 3.1)

project(http_server)

set(CMAKE_INSTALL_PREFIX ${CMAKE_HOME_DIRECTORY}/binaries/)

set(CMAKE_CXX_STANDARD 17)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#set(H_CONFIG_FILES )
find_package(OpenSSL)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(CPP_FILES
    httpserver.cpp
    api/api_handler.cpp
    users/users_storage.cpp
    jwt/json_web_tokens.cpp
    url/url.cpp)

add_executable(${PROJECT_NAME} ${CPP_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads ${OPENSSL_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

