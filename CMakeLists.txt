cmake_minimum_required(VERSION 3.30)

project(bootcamp LANGUAGES CXX)

option(UARTCOM "Use UART instead of TCP for communication" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

find_package(Threads REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia SerialPort)

set(CLIENT_DIR ${CMAKE_SOURCE_DIR}/desktop/client)
set(SERVER_DIR ${CMAKE_SOURCE_DIR}/desktop/server)
set(USER_FONTS_DIR $ENV{HOME}/.local/share/fonts)
set(FONT_FILENAME MaterialIcons.ttf)

if(NOT EXISTS ${USER_FONTS_DIR}/${FONT_FILENAME})
    message(STATUS "Copying ${FONT_FILENAME} -> ${USER_FONTS_DIR}/${FONT_FILENAME}")
    file(MAKE_DIRECTORY ${USER_FONTS_DIR})
    file(COPY_FILE ${CLIENT_DIR}/res/${FONT_FILENAME} ${USER_FONTS_DIR}/${FONT_FILENAME})
    execute_process(COMMAND fc-cache -f -v "${USER_FONTS_DIR}")
endif()

set(COMMON_SOURCES_BASENAMES
    comservice
    tcpservice
    uartservice
    window
)
foreach(basename IN LISTS COMMON_SOURCES_BASENAMES)
    string(TOUPPER ${basename} BASENAME)
    set(CLIENT_${BASENAME} ${CLIENT_DIR}/include/${basename}.h ${CLIENT_DIR}/src/${basename}.cpp)
    set(SERVER_${BASENAME} ${SERVER_DIR}/include/${basename}.h ${SERVER_DIR}/src/${basename}.cpp)
endforeach()

set(CLIENT_SOURCES
    ${CLIENT_DIR}/main.cpp
    ${CLIENT_DIR}/include/canvas.h ${CLIENT_DIR}/src/canvas.cpp
    ${CLIENT_WINDOW}
    ${CLIENT_COMSERVICE}
)
set(SERVER_SOURCES
    ${SERVER_DIR}/main.cpp
    ${SERVER_WINDOW}
    ${SERVER_COMSERVICE}
)
set(CLIENT_LIBS Threads::Threads Qt6::Core Qt6::Widgets Qt6::Multimedia)
set(SERVER_LIBS Threads::Threads Qt6::Core Qt6::Widgets)
set(CLIENT_DEFS)
set(SERVER_DEFS)

if(UARTCOM)
    set(CLIENT_UPORT "/dev/ttyUSB0" CACHE STRING "")
    set(SERVER_UPORT "/dev/ttyUSB1" CACHE STRING "")

    list(APPEND CLIENT_SOURCES ${CLIENT_UARTSERVICE})
    list(APPEND CLIENT_LIBS Qt6::SerialPort)
    list(APPEND CLIENT_DEFS UARTCOM UART_PORT="${CLIENT_UPORT}")

    list(APPEND SERVER_SOURCES ${SERVER_UARTSERVICE})
    list(APPEND SERVER_LIBS Qt6::SerialPort)
    list(APPEND SERVER_DEFS UARTCOM UART_PORT="${SERVER_UPORT}")

    add_custom_target(upload_client 
        COMMAND pio run -t upload --upload-port ${CLIENT_UPORT}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/esp32/client
        VERBATIM
    )
    add_custom_target(upload_server 
        COMMAND pio run -t upload --upload-port ${SERVER_UPORT}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/esp32/server
        VERBATIM
    )
else()
    list(APPEND CLIENT_SOURCES ${CLIENT_TCPSERVICE})
    list(APPEND SERVER_SOURCES ${SERVER_TCPSERVICE})
endif()

qt_standard_project_setup()

qt_add_executable(client ${CLIENT_SOURCES})
target_include_directories(client PRIVATE ${CLIENT_DIR}/include shared)
target_compile_definitions(client PRIVATE ${CLIENT_DEFS})
target_link_libraries(client PRIVATE ${CLIENT_LIBS})

qt_add_executable(server ${SERVER_SOURCES})
target_include_directories(server PRIVATE ${SERVER_DIR}/include shared)
target_compile_definitions(server PRIVATE ${SERVER_DEFS})
target_link_libraries(server PRIVATE ${SERVER_LIBS})

add_custom_target(use_tcp
    COMMAND ${CMAKE_COMMAND} -DUARTCOM:BOOL=OFF -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR}
    VERBATIM
)
add_custom_target(use_uart
    COMMAND ${CMAKE_COMMAND} -DUARTCOM:BOOL=ON -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR}
    VERBATIM
)