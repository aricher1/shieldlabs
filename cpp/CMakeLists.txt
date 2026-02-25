cmake_minimum_required(VERSION 3.15)
project(shieldlabs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# System Dependencies
# -----------------------------

set(SFML_STATIC_LIBRARIES ON)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)
find_package(yaml-cpp REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED poppler-cpp)

# -----------------------------
# Vendored ImGui
# -----------------------------

add_library(imgui
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp
)

target_include_directories(imgui PUBLIC
    external/imgui
)

# Tell imgui-sfml where imgui lives
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

# -----------------------------
# Vendored ImGui-SFML
# -----------------------------

add_subdirectory(external/imgui-sfml)

# -----------------------------
# Vendored nlohmann/json (header-only)
# -----------------------------

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/json/include
)

# -----------------------------
# Vendored NLopt
# -----------------------------

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(NLOPT_PYTHON OFF CACHE BOOL "" FORCE)
set(NLOPT_OCTAVE OFF CACHE BOOL "" FORCE)
set(NLOPT_MATLAB OFF CACHE BOOL "" FORCE)
set(NLOPT_GUILE OFF CACHE BOOL "" FORCE)
set(NLOPT_SWIG OFF CACHE BOOL "" FORCE)
set(NLOPT_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(external/nlopt)

# -----------------------------
# Executable
# -----------------------------

add_executable(shieldlabs
    src/ui/main.cpp
    src/ui/GridRenderer.cpp
    src/ui/GridMath.cpp
    src/ui/UiLog.cpp

    src/utils/PdfToPng.cpp
    src/utils/ProjectIO.cpp

    external/ImGuiFileDialog/ImGuiFileDialog.cpp

    src/geometry/GeometryEngine.cpp
    src/materials/MaterialRegistry.cpp

    src/calc/HitClassification.cpp
    src/calc/RayHitProcessing.cpp
    src/calc/RayIntersection.cpp
    src/calc/RayTraversal.cpp
    src/calc/SceneCompiler.cpp
    src/calc/TransportRay.cpp
    src/calc/CompilerOutput.cpp
    src/calc/EvaluateSingleRay.cpp
    src/calc/IntegrateDose.cpp
    src/calc/EvaluateRayPipeline.cpp

    src/optimization/ShieldOptimizer.cpp

    src/output/PrintCompilerOutput.cpp
    src/output/PrintCompilerOutputUI.cpp
    src/output/BuildDosePointReports.cpp
    src/output/ExportCompilerOutputCSV.cpp

    src/isotopes/IsotopeRegistry.cpp
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(shieldlabs PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# -----------------------------
# Assets Copy
# -----------------------------

add_custom_command(TARGET shieldlabs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:shieldlabs>/assets
)

# -----------------------------
# Includes
# -----------------------------

target_include_directories(shieldlabs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tabulate/include
    ${POPPLER_CPP_INCLUDE_DIRS}
)

target_compile_options(shieldlabs PRIVATE
    ${POPPLER_CPP_CFLAGS_OTHER}
)

# -----------------------------
# Link Libraries
# -----------------------------

target_link_libraries(shieldlabs PRIVATE
    imgui
    ImGui-SFML
    nlohmann_json
    nlopt
    SFML::Graphics
    SFML::Window
    SFML::System
    yaml-cpp
    ${POPPLER_CPP_LIBRARIES}
)