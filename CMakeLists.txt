cmake_minimum_required(VERSION 3.1)

project(da_gis_service)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/binaries/)

set(CMAKE_CXX_STANDARD 17)

message(STATUS "${CMAKE_SYSTEM_PROCESSOR}")

#set(H_CONFIG_FILES )

set(CPP_FILES
    main.cpp)

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "X86_64")
    add_subdirectory(wxUI)
endif()

add_subdirectory(websocket_srv)
add_subdirectory(http_server)
add_subdirectory(test)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh ${CMAKE_INSTALL_PREFIX}/run.sh COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/stop.sh ${CMAKE_INSTALL_PREFIX}/stop.sh COPYONLY)
