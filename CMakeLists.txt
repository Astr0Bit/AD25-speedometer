cmake_minimum_required(VERSION 3.30)

project(bootcamp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia SerialPort)

set(CLIENT_DIR desktop/client)
set(SERVER_DIR desktop/server)
set(ESP32_CLIENT_DIR esp32/client)
set(ESP32_SERVER_DIR esp32/server)
set(FONT_FILENAME MaterialIcons.ttf)
set(USER_FONTS_DIR $ENV{HOME}/.local/share/fonts)

if(NOT EXISTS ${USER_FONTS_DIR}/${FONT_FILENAME})
    message(STATUS "Copying ${FONT_FILENAME} -> ${USER_FONTS_DIR}/${FONT_FILENAME}")
    execute_process(
        COMMAND mkdir -p "${USER_FONTS_DIR}"
        COMMAND cp "${CLIENT_DIR}/res/${FONT_FILENAME}" "${USER_FONTS_DIR}/${FONT_FILENAME}"
        COMMAND fc-cache -f -v "${USER_FONTS_DIR}"
    )
endif()

set(CLIENT_GUI_SRC
    ${CLIENT_DIR}/main.cpp
    ${CLIENT_DIR}/src/window.cpp
    ${CLIENT_DIR}/src/canvas.cpp
)
set(SERVER_GUI_SRC
    ${SERVER_DIR}/main.cpp
    ${SERVER_DIR}/src/window.cpp
)

set(COM_SRC_FILENAMES
    comservice.cpp
    tcpservice.cpp
    uartservice.cpp
)
foreach(cpp IN LISTS COM_SRC_FILENAMES)
    list(APPEND CLIENT_COM_SRC ${CLIENT_DIR}/src/${cpp})
    list(APPEND SERVER_COM_SRC ${SERVER_DIR}/src/${cpp})
endforeach()

qt_standard_project_setup()

qt_add_library(client-comservice STATIC ${CLIENT_COM_SRC})
target_include_directories(client-comservice PRIVATE ${CLIENT_DIR}/include shared)
target_link_libraries(client-comservice PRIVATE Qt6::Core Qt6::SerialPort)

qt_add_library(server-comservice STATIC ${SERVER_COM_SRC})
target_include_directories(server-comservice PRIVATE ${SERVER_DIR}/include shared)
target_link_libraries(server-comservice PRIVATE Qt6::Core Qt6::SerialPort)

qt_add_executable(client ${CLIENT_GUI_SRC})
target_include_directories(client PRIVATE ${CLIENT_DIR}/include shared)
target_link_libraries(client PRIVATE Qt6::Core Qt6::Widgets Qt6::Multimedia client-comservice)

qt_add_executable(server ${SERVER_GUI_SRC})
target_include_directories(server PRIVATE ${SERVER_DIR}/include shared)
target_link_libraries(server PRIVATE Qt6::Core Qt6::Widgets server-comservice)
