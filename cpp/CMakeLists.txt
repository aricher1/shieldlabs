cmake_minimum_required(VERSION 3.15)
project(shieldlabs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
add_compile_definitions(NDEBUG)

# SFML
set(SFML_STATIC_LIBRARIES ON)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

# yaml-cpp
find_package(yaml-cpp REQUIRED)

# Poppler
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED poppler-cpp)

# NLopt
find_package(NLopt REQUIRED PATHS /usr/local)

# nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)

FetchContent_MakeAvailable(imgui)
set(IMGUI_DIR ${imgui_SOURCE_DIR})

# ImGui-SFML
FetchContent_Declare(
    imgui_sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG master
)

FetchContent_MakeAvailable(imgui_sfml)
FetchContent_MakeAvailable(nlohmann_json)

# Executable
add_executable(shieldlabs
    # ui
    src/ui/main.cpp
    src/ui/GridRenderer.cpp
    src/ui/GridMath.cpp
    src/ui/UiLog.cpp
    
    # utils
    src/utils/PdfToPng.cpp
    src/utils/ProjectIO.cpp
    
    # external
    external/ImGuiFileDialog/ImGuiFileDialog.cpp
    
    # geometry
    src/geometry/GeometryEngine.cpp
    
    # materials
    src/materials/MaterialRegistry.cpp
    
    # calc
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
    
    # optimization
    src/optimization/ShieldOptimizer.cpp
    
    # output
    src/output/PrintCompilerOutput.cpp
    src/output/PrintCompilerOutputUI.cpp
    src/output/BuildDosePointReports.cpp
    src/output/ExportCompilerOutputCSV.cpp
    
    # isotopes
    src/isotopes/IsotopeRegistry.cpp
)

# Assets
add_custom_command(TARGET shieldlabs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:shieldlabs>/assets
)

# Includes
target_include_directories(shieldlabs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tabulate/include
    ${POPPLER_CPP_INCLUDE_DIRS}
)

target_compile_options(shieldlabs PRIVATE
    ${POPPLER_CPP_CFLAGS_OTHER}
)

# Link libraries
target_link_libraries(shieldlabs PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    ImGui-SFML::ImGui-SFML
    nlohmann_json::nlohmann_json
    yaml-cpp
    NLopt::nlopt
    ${POPPLER_CPP_LIBRARIES}
)
