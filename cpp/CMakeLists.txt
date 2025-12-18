cmake_minimum_required(VERSION 3.15)
project(xrct_geometry)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

include_directories(include external)

add_executable(xrct_ui
    src/ui/main.cpp
    src/ui/GridRenderer.cpp
    src/GeometryEngine.cpp
)

target_include_directories(xrct_ui PRIVATE
    ${CMAKE_SOURCE_DIR}/cpp/include
    ${CMAKE_SOURCE_DIR}/cpp/external
)

target_link_libraries(xrct_ui
    SFML::Graphics
    SFML::Window
    SFML::System
)