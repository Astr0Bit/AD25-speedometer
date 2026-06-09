cmake_minimum_required(VERSION 3.22)

project(server_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets SerialPort)

# NOTE: Could be improved, but works for now
set(
    SOURCES
    main.cpp
    include/window.h
    include/comservice.h
    include/tcpservice.h
    include/uartservice.h
    src/comservice.cpp
    src/tcpservice.cpp
    src/uartservice.cpp
    src/window.cpp
)

add_library(shared INTERFACE)
target_include_directories(shared INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../shared)

add_executable(server_gui ${SOURCES})
target_include_directories(server_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(server_gui PRIVATE shared Qt6::Core Qt6::Widgets Qt6::SerialPort)
