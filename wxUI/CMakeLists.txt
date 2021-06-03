cmake_minimum_required(VERSION 3.1)

project(wxMonitor)

set(CMAKE_INSTALL_PREFIX ${CMAKE_HOME_DIRECTORY}/binaries/)

set(CMAKE_CXX_STANDARD 17)

SET(wxWidgets_USE_LIBS)
FIND_PACKAGE(wxWidgets REQUIRED)
INCLUDE("${wxWidgets_USE_FILE}" "/usr/local/lib/wx/include/gtk3-unicode-3.1")

#set(H_CONFIG_FILES )

set(CPP_FILES 
    wxMain.cpp
    websocket_client.cpp
)

add_executable(${PROJECT_NAME} ${CPP_FILES})

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${wxWidgets_LIBRARIES} X11 "stdc++fs" )

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

