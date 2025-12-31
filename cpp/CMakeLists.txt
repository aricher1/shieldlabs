cmake_minimum_required(VERSION 3.15)
project(xrct_geometry)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(SFML_STATIC_LIBRARIES ON)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)
find_package(yaml-cpp REQUIRED)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)

add_executable(xrct_ui
    src/ui/main.cpp
    src/ui/GridRenderer.cpp
    src/geometry/GeometryEngine.cpp
    src/materials/MaterialRegistry.cpp
    src/calc/HitClassification.cpp
    src/calc/RayHitProcessing.cpp
    src/calc/RayIntersection.cpp
    src/calc/RayTraversal.cpp
    src/calc/SceneCompiler.cpp
    src/calc/TransportRay.cpp
    src/calc/CompilerOutput.cpp
    src/output/PrintCompilerOutput.cpp
    src/isotopes/IsotopeRegistry.cpp

)

add_custom_command(TARGET xrct_ui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:xrct_ui>/assets
)

target_include_directories(xrct_ui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)

target_link_libraries(xrct_ui PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::System
    nlohmann_json::nlohmann_json
    yaml-cpp
)
