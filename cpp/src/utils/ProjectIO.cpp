// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

#include "utils/ProjectIO.hpp"
#include "geometry/GeometryEngine.hpp"


using namespace geom;


namespace utils {

    bool save_project(const GeometryEngine& engine, const std::string& project_dir, const std::string& floorplan_png_path) {
        namespace fs = std::filesystem;

        if (floorplan_png_path.empty()) {
            return false;
        }

        fs::create_directories(project_dir);

        fs::copy_file(
            floorplan_png_path,
            fs::path(project_dir) / "floorplan.png",
            fs::copy_options::overwrite_existing
        );

        nlohmann::json j;
        j["version"] = 1;
        j["floorplan_image"] = "floorplan.png";
        j["geometry"] = engine.to_json();

        std::ofstream out(fs::path(project_dir) / "project.json");
        if (!out.is_open()) { return false; }

        out << j.dump(2);
        return true;
    }


    bool load_project(GeometryEngine& engine, const std::string& project_dir, std::string& out_floorplan_png_path) {
        namespace fs = std::filesystem;

        std::ifstream in(fs::path(project_dir) / "project.json");
        if (!in.is_open()) return false;

        nlohmann::json j;
        in >> j;

        std::string image_name = j["floorplan_image"];
        out_floorplan_png_path = (fs::path(project_dir) / image_name).string();

        engine.clear();
        engine.load_from_json(j["geometry"]);

        return true;
    }

} // end namespace utils