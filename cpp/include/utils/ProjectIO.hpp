#pragma once
#include "geometry/GeometryEngine.hpp"
#include <string>


bool save_project(const GeometryEngine& engine, const std::string& project_dir, const std::string& floorplan_png_path);

bool load_project(GeometryEngine& engine, const std::string& project_dir, std::string& out_floorplan_png_path);