// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <cmath>
#include <algorithm>
#include <iostream>

#include "calc/SceneCompiler.hpp"


namespace calc {

    CalcScene SceneCompiler::compile(const nlohmann::json& j) {
        CalcScene scene;

        const double scale = j.value("distance_scale", 1.0);
        scene.isotope_key = j.value("isotope", "f18");

        // walls
        if (j.contains("walls")) {
            for (const auto& jw : j["walls"]) {
                CalcWall wall;

                wall.a = {jw["a"]["x"].get<double>() * scale, jw["a"]["y"].get<double>() * scale};
                wall.b = {jw["b"]["x"].get<double>() * scale, jw["b"]["y"].get<double>() * scale};

                // wall length
                const double dx = wall.b.x_cm - wall.a.x_cm;
                const double dy = wall.b.y_cm - wall.a.y_cm;
                wall.length_cm = std::sqrt(dx * dx + dy * dy);

                // ===== SCALE SANITY CHECK ==============================================================================
                // std::cout << "SCALE SANITY CHECK\n";
                // std::cout << "distance_scale = " << scale << "\n";
                // std::cout << "Wall A raw: (" << jw["a"]["x"].get<double>() << ", " << jw["a"]["y"].get<double>() << ")\n";
                // std::cout << "Wall B raw: (" << jw["b"]["x"].get<double>() << ", " << jw["b"]["y"].get<double>() << ")\n";
                // std::cout << "Wall A scaled: (" << wall.a.x_cm << ", " << wall.a.y_cm << ")\n";
                // std::cout << "Wall B scaled: (" << wall.b.x_cm << ", " << wall.b.y_cm << ")\n";
                // std::cout << "Wall length (cm): " << wall.length_cm << "\n";
                // =======================================================================================================

                if (jw.contains("layers")) {
                    for (const auto& jl : jw["layers"]) {
                        wall.layers.push_back({jl["material_id"].get<int>(), jl["thickness_cm"].get<double>()});
                    }
                }

                scene.walls.push_back(wall);
            }
        }

        // sources
        if (j.contains("sources")) {
            for (const auto& js : j["sources"]) {
                CalcSource src;
                src.position = {js["x"].get<double>() * scale, js["y"].get<double>() * scale};
                src.num_patients = js["num_patients"].get<double>();
                src.activity_per_patient_MBq = js["activity_per_patient_MBq"].get<double>();
                src.uptake_time_hours = js["uptake_time_hours"].get<double>();
                src.apply_patient_attenuation = js.value("apply_patient_attenuation", false);
                src.patient_attenuation_percent = js.value("patient_attenuation_percent", 0.0f);
                src.apply_radioactive_decay = js.value("apply_radioactive_decay", false);
                src.label = js.value("label", "");

                scene.sources.push_back(src);
            }
        }

        // dose points
        if (j.contains("dose_points")) {
            for (const auto& jd : j["dose_points"]) {
                CalcDosePoint dp;
                dp.position = {jd["x"].get<double>() * scale, jd["y"].get<double>() * scale};
                dp.occupancy = jd.value("occupancy", 1.0);
                dp.dose_limit_uSv = jd.value("dose_limit_uSv", 0.0);
                dp.label = jd.value("label", "");

                scene.dose_points.push_back(dp);
            }
        }

        return scene;
    }
        
} // end namespace calc