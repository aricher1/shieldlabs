cmake_minimum_required(VERSION 3.15)
project(xrct_geometry)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(xrct_geom
    src/main.cpp
    src/GeometryEngine.cpp    
)