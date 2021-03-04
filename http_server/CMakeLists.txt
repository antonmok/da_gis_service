cmake_minimum_required(VERSION 3.1)

project(http_server)

set(CMAKE_INSTALL_PREFIX ${CMAKE_HOME_DIRECTORY}/binaries/)

set(CMAKE_CXX_STANDARD 17)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#set(H_CONFIG_FILES )
find_package(OpenSSL)
find_package (glog 0.5.0 REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})

set(CPP_FILES
    api/api_handler.cpp
    users/users_storage.cpp
    jwt/json_web_tokens.cpp
    url/url.cpp)

set(CPP_TEST_FILES
    api/test_api.cpp
    users/test_users.cpp
    jwt/test_jwt.cpp)

add_executable(${PROJECT_NAME} httpserver.cpp ${CPP_FILES})
add_executable(${PROJECT_NAME}_test ${CPP_FILES} ${CPP_TEST_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads glog::glog ${OPENSSL_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_test PRIVATE Threads::Threads glog::glog ${OPENSSL_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
