cmake_minimum_required(VERSION 3.1)

project(da_gis_service)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/binaries/)

set(CMAKE_CXX_STANDARD 17)

SET(wxWidgets_USE_LIBS)
FIND_PACKAGE(wxWidgets)
INCLUDE("${wxWidgets_USE_FILE}")

#set(H_CONFIG_FILES )

set(CPP_FILES
    main.cpp)

add_subdirectory(wxUI)
add_subdirectory(websocket_srv)

add_executable(${PROJECT_NAME} ${CPP_FILES})

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${wxWidgets_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

