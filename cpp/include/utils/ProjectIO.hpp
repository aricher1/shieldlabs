#pragma once

#include <string>

#include "geometry/GeometryEngine.hpp"



namespace utils {

    bool save_project(const geom::GeometryEngine& engine, const std::string& project_dir, const std::string& floorplan_png_path);

    bool load_project(geom::GeometryEngine& engine, const std::string& project_dir, std::string& out_floorplan_png_path);

} // end namespace utils