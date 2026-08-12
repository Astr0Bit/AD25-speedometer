cmake_minimum_required(VERSION 3.30)

project(bootcamp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

find_package(Threads)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia SerialPort)

set(CLIENT_DIR ${CMAKE_SOURCE_DIR}/desktop/client)
set(SERVER_DIR ${CMAKE_SOURCE_DIR}/desktop/server)
set(ESP32_CLIENT_DIR ${CMAKE_SOURCE_DIR}/esp32/client)
set(ESP32_SERVER_DIR ${CMAKE_SOURCE_DIR}/esp32/server)
set(USER_FONTS_DIR $ENV{HOME}/.local/share/fonts)
set(FONT_FILENAME MaterialIcons.ttf)

if(NOT EXISTS ${USER_FONTS_DIR}/${FONT_FILENAME})
    message(STATUS "Copying ${FONT_FILENAME} -> ${USER_FONTS_DIR}/${FONT_FILENAME}")
    execute_process(
        COMMAND mkdir -p "${USER_FONTS_DIR}"
        COMMAND cp "${CLIENT_DIR}/res/${FONT_FILENAME}" "${USER_FONTS_DIR}/${FONT_FILENAME}"
        COMMAND fc-cache -f -v "${USER_FONTS_DIR}"
    )
endif()

set(COM_SRC_FILENAMES
    comservice.cpp
    tcpservice.cpp
    uartservice.cpp
)
foreach(cpp IN LISTS COM_SRC_FILENAMES)
    get_filename_component(cppbase ${cpp} NAME_WE)
    string(TOUPPER ${cppbase} CPPBASE)
    set(CLIENT_${CPPBASE} ${CLIENT_DIR}/src/${cpp})
    set(SERVER_${CPPBASE} ${SERVER_DIR}/src/${cpp})
endforeach()

qt_standard_project_setup()

qt_add_library(client-comservice STATIC ${CLIENT_COMSERVICE})
target_include_directories(client-comservice PUBLIC ${CLIENT_DIR}/include shared)
qt_add_library(server-comservice STATIC ${SERVER_COMSERVICE})
target_include_directories(server-comservice PUBLIC ${SERVER_DIR}/include shared)

qt_add_library(client-tcpservice STATIC ${CLIENT_TCPSERVICE})
target_link_libraries(client-tcpservice PUBLIC client-comservice Qt6::Core)
qt_add_library(server-tcpservice STATIC ${SERVER_TCPSERVICE})
target_link_libraries(server-tcpservice PUBLIC server-comservice Qt6::Core)

qt_add_library(client-uartservice STATIC ${CLIENT_UARTSERVICE})
target_compile_definitions(client-uartservice PUBLIC UARTCOM)
target_link_libraries(client-uartservice PUBLIC client-comservice Qt6::Core Qt6::SerialPort)
qt_add_library(server-uartservice STATIC ${SERVER_UARTSERVICE})
target_compile_definitions(server-uartservice PUBLIC UARTCOM)
target_link_libraries(server-uartservice PUBLIC server-comservice Qt6::Core Qt6::SerialPort)

set(CLIENT_GUI_SRC 
    ${CLIENT_DIR}/main.cpp
    ${CLIENT_DIR}/src/window.cpp
    ${CLIENT_DIR}/src/canvas.cpp
)
set(SERVER_GUI_SRC 
    ${SERVER_DIR}/main.cpp
    ${SERVER_DIR}/src/window.cpp
)
set(COMMON_GUI_LIBS 
    Qt6::Core
    Qt6::Widgets
    Threads::Threads
)

qt_add_executable(client-tcp ${CLIENT_GUI_SRC})
target_link_libraries(client-tcp PRIVATE 
    ${COMMON_GUI_LIBS}
    Qt6::Multimedia
    client-tcpservice
)
qt_add_executable(server-tcp ${SERVER_GUI_SRC})
target_link_libraries(server-tcp PRIVATE
    ${COMMON_GUI_LIBS}
    server-tcpservice
)

qt_add_executable(client-uart ${CLIENT_GUI_SRC})
target_link_libraries(client-uart PRIVATE 
    ${COMMON_GUI_LIBS}
    Qt6::Multimedia
    client-uartservice
)
qt_add_executable(server-uart ${SERVER_GUI_SRC})
target_link_libraries(server-uart PRIVATE
    ${COMMON_GUI_LIBS}
    server-uartservice
)

add_custom_target(tcp)
add_dependencies(tcp client-tcp server-tcp)

add_custom_target(upload-client 
    COMMAND pio run -t upload
    WORKING_DIRECTORY ${ESP32_CLIENT_DIR}
    VERBATIM
)
add_custom_target(upload-server 
    COMMAND pio run -t upload
    WORKING_DIRECTORY ${ESP32_SERVER_DIR}
    VERBATIM
)
