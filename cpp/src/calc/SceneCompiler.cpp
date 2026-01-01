#include "calc/SceneCompiler.hpp"
#include <cmath>
#include <algorithm>



namespace calc {

CalcScene SceneCompiler::compile(const nlohmann::json& j) {
    CalcScene scene;

    // walls
    if (j.contains("walls")) {
        for (const auto& jw : j["walls"]) {
            CalcWall wall;

            wall.a = {jw["a"]["x"].get<double>(), jw["a"]["y"].get<double>()};
            wall.b = {jw["b"]["x"].get<double>(), jw["b"]["y"].get<double>()};

            // wall length
            const double dx = wall.b.x_cm - wall.a.x_cm;
            const double dy = wall.b.y_cm - wall.a.y_cm;
            wall.length_cm = std::sqrt(dx * dx + dy * dy);

            if (jw.contains("layers")) {
                for (const auto& jl : jw["layers"]) {
                    wall.layers.push_back({jl["material_id"].get<int>(), jl["thickness_cm"].get<double>()});
                }
            }

            if (jw.contains("openings")) {
                for (const auto& jo : jw["openings"]) {
                    const double center_t = jo["center_t"].get<double>();
                    const double length_cm = jo["length_cm"].get<double>();
                    const double half_t = length_cm / (2.0 * wall.length_cm);

                    CalcOpening opening;
                    opening.t0 = std::clamp(center_t - half_t, 0.0, 1.0);
                    opening.t1 = std::clamp(center_t + half_t, 0.0, 1.0);

                    const std::string type = jo["type"].get<std::string>();
                    if (type == "door") {
                        opening.type = OpeningType::Door;
                    } else if (type == "window") {
                        opening.type = OpeningType::Window;
                    } else {
                        opening.type = OpeningType::Open;
                    }

                    wall.openings.push_back(opening);
                }
            }

            scene.walls.push_back(wall);
        }
    }

    // sources
    if (j.contains("sources")) {
        for (const auto& js : j["sources"]) {
            CalcSource src;
            src.position = {js["x"].get<double>(), js["y"].get<double>()};
            src.num_patients = js["num_patients"].get<double>();
            src.activity_per_patient_MBq = js["activity_per_patient_MBq"].get<double>();
            src.uptake_time_hours = js["uptake_time_hours"].get<double>();
            src.apply_patient_attenuation = js.value("apply_patient_attenuation", false);
            src.apply_radioactive_decay = js.value("apply_radioactive_decay", false);

            scene.sources.push_back(src);
        }
    }

    // dose points
    if (j.contains("dose_points")) {
        for (const auto& jd : j["dose_points"]) {
            scene.dose_points.push_back({{ jd["x"].get<double>(), jd["y"].get<double>() }});
        }
    }

    return scene;
}
    

} // end namespace calc