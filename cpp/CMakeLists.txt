# ============================================================================================
#      _____ __  ________________   ____  __    ___    ____  _____
#     / ___// / / /  _/ ____/ / /  / __ \/ /   /   |  / __ // ___/
#     \__ \/ /_/ // // __/ / /    / / / / /   / /| | / __  |\__ \ 
#    ___/ / __  // // /___/ /___ / /_/ / /___/ ___ |/ /_/ /___/ / 
#   /____/_/ /_/___/_____/_____//_____/_____/_/  |_/____//____/
#
# ============================================================================================

cmake_minimum_required(VERSION 3.15)
project(shieldlabs)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ============================================================================================
# System Dependencies
# ============================================================================================
set(POPPLER_CPP_TARGET "")
set(POPPLER_CPP_INCLUDE_DIRS "")
set(POPPLER_CPP_CFLAGS_OTHER "")

if(WIN32)
    find_package(unofficial-poppler CONFIG QUIET)
    find_package(poppler CONFIG QUIET)
    find_package(poppler-cpp CONFIG QUIET)

    if(TARGET unofficial::poppler::poppler-cpp)
        set(POPPLER_CPP_TARGET unofficial::poppler::poppler-cpp)
    elseif(TARGET poppler-cpp)
        set(POPPLER_CPP_TARGET poppler-cpp)
    elseif(TARGET Poppler::Cpp)
        set(POPPLER_CPP_TARGET Poppler::Cpp)
    else()
        message(FATAL_ERROR "Could not find a Poppler C++ target on Windows. Configure CMake with the vcpkg toolchain and install Poppler.")
    endif()
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(POPPLER_CPP REQUIRED IMPORTED_TARGET poppler-cpp)
    set(POPPLER_CPP_TARGET PkgConfig::POPPLER_CPP)
endif()

# ============================================================================================
# Vendored SFML
# ============================================================================================
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_USE_SYSTEM_DEPS ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/sfml)

# ============================================================================================
# Vendored ImGui
# ============================================================================================
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

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

# ============================================================================================
# Vendored ImGui-SFML
# ============================================================================================
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
add_subdirectory(external/imgui-sfml)

# ============================================================================================
# Vendored nlohmann/json (header-only)
# ============================================================================================
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/json/include
)

# ============================================================================================
# Vendored NLopt
# ============================================================================================
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(NLOPT_PYTHON OFF CACHE BOOL "" FORCE)
set(NLOPT_OCTAVE OFF CACHE BOOL "" FORCE)
set(NLOPT_MATLAB OFF CACHE BOOL "" FORCE)
set(NLOPT_GUILE OFF CACHE BOOL "" FORCE)
set(NLOPT_SWIG OFF CACHE BOOL "" FORCE)
set(NLOPT_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/nlopt)

# ============================================================================================
# Vendored yaml-cpp
# ============================================================================================
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
add_subdirectory(external/yaml-cpp)

# ============================================================================================
# Executable
# ============================================================================================
set(SHIELDLABS_SOURCES
    src/ui/main.cpp
    src/ui/GridRenderer.cpp
    src/ui/GridMath.cpp
    src/ui/UiLog.cpp

    src/utils/AppPaths.cpp
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

if(WIN32)
    list(APPEND SHIELDLABS_SOURCES
        resources/shieldlabs.rc
    )
endif()

add_executable(shieldlabs
    ${SHIELDLABS_SOURCES}
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(shieldlabs PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# ============================================================================================
# Platform Fixes
# ============================================================================================
target_compile_definitions(shieldlabs PRIVATE
    $<$<PLATFORM_ID:Windows>:NOMINMAX>
)

if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

# ============================================================================================
# Assets Copy
# ============================================================================================
add_custom_command(TARGET shieldlabs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:shieldlabs>/assets
)

# ============================================================================================
# Includes
# ============================================================================================
target_include_directories(shieldlabs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tabulate/include
    ${POPPLER_CPP_INCLUDE_DIRS}
)

target_compile_options(shieldlabs PRIVATE
    ${POPPLER_CPP_CFLAGS_OTHER}
)

# ============================================================================================
# Link Libraries
# ============================================================================================
target_link_libraries(shieldlabs PRIVATE
    imgui
    ImGui-SFML
    nlohmann_json
    nlopt
    sfml-graphics
    sfml-window
    sfml-system
    yaml-cpp::yaml-cpp
    ${POPPLER_CPP_TARGET}
)
