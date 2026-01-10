#include "geometry/GeometryEngine.hpp"
#include <nlohmann/json.hpp>
#include <cmath>
#include <map>

using json = nlohmann::json;

constexpr double SNAP_EPS_CM = 1e-3;


GeometryEngine::GeometryEngine(int cells, double cm) : grid_cells(cells), cm_per_cell(cm) {
    world_bounds.width_cm = grid_cells * cm_per_cell;
    world_bounds.height_cm = grid_cells * cm_per_cell;
}


Point GeometryEngine::snap_to_grid(Point p) const {
    
    Point snapped{std::round(p.x_cm / cm_per_cell) * cm_per_cell, std::round(p.y_cm / cm_per_cell) * cm_per_cell};
    snapped.x_cm = std::clamp(snapped.x_cm, 0.0, world_bounds.width_cm);
    snapped.y_cm = std::clamp(snapped.y_cm, 0.0, world_bounds.height_cm);

    return snapped;
}


Point GeometryEngine::reuse_or_add(Point p) {

    for (const auto& existing : points) {
        double dx = existing.x_cm - p.x_cm;
        double dy = existing.y_cm - p.y_cm;

        if (std::hypot(dx, dy) < SNAP_EPS_CM) {
            return existing;
        }
    }
    points.push_back(p);    
    
    return p;
}


Point GeometryEngine::add_point(Point p) {
    p = snap_to_grid(p);
    return reuse_or_add(p);
}


void GeometryEngine::add_wall(Point a, Point b, double thickness_cm, int material_id) {
    a = add_point(a);
    b = add_point(b);
    const double length_cm = std::hypot(a.x_cm - b.x_cm, a.y_cm - b.y_cm);

    if (length_cm < SNAP_EPS_CM) { 
        return; 
    }

    Wall w;
    w.a = a;
    w.b = b;
    w.length_cm = length_cm;
    w.layers.push_back({material_id, thickness_cm});
    walls.push_back(w);
}


void GeometryEngine::add_wall_direct(const Wall& w) {
    walls.push_back(w);
}


void GeometryEngine::remove_last_wall() {
    if(!walls.empty()) {
        walls.pop_back();
    }
}


void GeometryEngine::remove_wall_at(std::size_t index) {
    if (index < walls.size()) {
        walls.erase(walls.begin() + index);
    }
}


void GeometryEngine::remove_entity_at(std::size_t index) {
    if (index < entities.size()) {
        entities.erase(entities.begin() + index);
    }
}


void GeometryEngine::add_entity_direct(const PointEntity& e) {
    entities.push_back(e);
}


const std::vector<Wall>& GeometryEngine::get_walls() const { return walls; }


void GeometryEngine::add_source(Point p) {
    p = snap_to_grid(p);

    PointEntity e;
    e.position = p;
    e.type = PointType::Source;
    e.label = "";
    e.source = SourceData{
        .num_patients = 1.0f,
        .activity_per_patient_MBq = 0.0f,
        .uptake_time_hours = 0.0f
    };

    entities.push_back(e);
}


void GeometryEngine::add_dose(Point p) {
    p = snap_to_grid(p);

    PointEntity e;
    e.position = p;
    e.type = PointType::Dose;
    e.label = "";
    e.dose = DoseData{
        .occupancy = 1.0f,
        .occupancy_type = "",
        .dose_limit_uSv = 0.0f
    };

    entities.push_back(e);
}


void GeometryEngine::set_distance_scale(double scale) {
    if (scale > 0.0) {
        distance_scale = scale;
    }
}


const std::vector<PointEntity>& GeometryEngine::get_entities() const { return entities; }


nlohmann::json GeometryEngine::to_json() const {

    json j;

    j["version"] = 2;
    j["units"] = "cm";

    j["grid"] = {
        {"cells", grid_cells},
        {"cm_per_cell", cm_per_cell}
    };

    j["distance_scale"] = distance_scale;
    j["walls"] = json::array();
    
    for (const auto& w : walls) {
        
        json jw;
        jw["a"] = {{"x", w.a.x_cm}, {"y", w.a.y_cm}};
        jw["b"] = {{"x", w.b.x_cm}, {"y", w.b.y_cm}};
        jw["layers"] = json::array();
        for (const auto& layer : w.layers) {
            jw["layers"].push_back({{"material_id", layer.material_id}, {"thickness_cm", layer.thickness_cm}});
        }
        jw["length_cm"] = w.length_cm * distance_scale;
        
        // openings
        jw["openings"] = json::array();
        for (const auto& o : w.openings) {
            json jo;
            switch (o.type) {
                case OpeningType::Door: jo["type"] = "door"; break;
                case OpeningType::Window: jo["type"] = "window"; break;
                case OpeningType::Open: jo["type"] = "open"; break; 
            }

            jo["center_t"] = o.center_t;
            jo["length_cm"] = o.length_cm * distance_scale;
            jw["openings"].push_back(jo);
        }

        j["walls"].push_back(jw);
    }

    j["sources"] = json::array();
    j["dose_points"] = json::array();

    for (const auto& e : entities) {
        json entry;
        entry["x"] = e.position.x_cm;
        entry["y"] = e.position.y_cm;

        if (!e.label.empty()) {
            entry["label"] = e.label;
        }

        // source points
        if (e.type == PointType::Source && e.source.has_value()) {
            const auto& s = e.source.value();
            entry["num_patients"] = s.num_patients;
            entry["activity_per_patient_MBq"] = s.activity_per_patient_MBq;
            entry["uptake_time_hours"] = s.uptake_time_hours;
            entry["apply_patient_attenuation"] = s.apply_patient_attenuation;
            entry["apply_radioactive_decay"] = s.apply_radioactive_decay;

            j["sources"].push_back(entry);
        }

        // dose points
        if (e.type == PointType::Dose && e.dose.has_value()) {
            const auto& d = e.dose.value();
            entry["occupancy"] = d.occupancy;
            entry["occupancy_type"] = d.occupancy_type;
            entry["dose_limit_uSv"] = d.dose_limit_uSv;

            j["dose_points"].push_back(entry);
        }
    }

    return j;
}


bool GeometryEngine::load_from_json(const std::string& json_str) {

    json j;
    try {
        j = json::parse(json_str);
    } catch (...) {
        return false;
    }

    if (!j.contains("version")) { return false; }
    int version = j["version"];
    if (version != 1 && version != 2) { return false; }
    if (!j.contains("grid")) { return false; }

    grid_cells  = j["grid"]["cells"];
    cm_per_cell = j["grid"]["cm_per_cell"];
    distance_scale = j["distance_scale"];
    clear();
    entities.clear();

    if (j.contains("walls")) {

        for (const auto& jw : j["walls"]) {

            Point a{jw["a"]["x"], jw["a"]["y"]};
            Point b{jw["b"]["x"], jw["b"]["y"]};

            Wall w;
            w.a = add_point(a);
            w.b = add_point(b);
            w.length_cm = std::hypot(w.a.x_cm - w.b.x_cm, w.a.y_cm - w.b.y_cm);

            if (version == 1) {
                w.layers.push_back({jw["material_id"], jw["thickness_cm"]});
            }

            if (version == 2 && jw.contains("layers")) {
                for (const auto& jl : jw["layers"]) {
                    w.layers.push_back({jl["material_id"], jl["thickness_cm"]});
                }
            }
            walls.push_back(w);

            if (jw.contains("openings")) {
                Wall& wref = walls.back();
                for (const auto& jo : jw["openings"]) {
                    WallOpening o;
                    const std::string type = jo["type"];
                    if (type == "door") {
                        o.type = OpeningType::Door;
                    } else if (type == "window") {
                        o.type = OpeningType::Window;
                    } else {
                        o.type = OpeningType::Open;
                    }
                    o.center_t = jo["center_t"];
                    o.length_cm = jo["length_cm"];
                    wref.openings.push_back(o);
                }
            }
        }
    }

    // source points
    if (j.contains("sources")) {
        for (const auto& s : j["sources"]) {
            PointEntity e;
            e.position = {s["x"], s["y"]};
            e.type = PointType::Source;
            e.label = s.value("label", "");
            e.source = SourceData{
                .num_patients = s.value("num_patients", 1.0f),
                .activity_per_patient_MBq = s.value("activity_per_patient_MBq", 0.0f),
                .uptake_time_hours = s.value("uptake_time_hours", 0.0f)
            };
        
            entities.push_back(e);
        }
    }

    // dose points
    if (j.contains("dose_points")) {
        for (const auto& d : j["dose_points"]) {
            PointEntity e;
            e.position = {d["x"], d["y"]};
            e.type = PointType::Dose;
            e.label = d.value("label", "");
            e.dose = DoseData{
                .occupancy = d.value("occupancy", 1.0f),
                .occupancy_type = d.value("occupancy_type", ""),
                .dose_limit_uSv = d.value("dose_limit_uSv", 0.0f)
            };

            entities.push_back(e);
        }
    }

    return true;
}


std::vector<GeometryEngine::ValidationError> GeometryEngine::validate() const {
    std::vector<ValidationError> errors; // create vector to store all errors

    // check walls
    for (std::size_t i = 0; i < walls.size(); ++i) {
        const auto& w = walls[i];

        // check wall enpoints are inside grid
        auto check_point_in_bounds = [&](const Point& p, const std::string& label) {
            if (p.x_cm < 0.0 || p.x_cm > world_bounds.width_cm || p.y_cm < 0.0 || p.y_cm > world_bounds.height_cm) {
                errors.push_back({label + " is outside grid bounds."});
            }
        };

        check_point_in_bounds(w.a, "Wall " + std::to_string(i) + " endpoint A.");
        check_point_in_bounds(w.b, "Wall " + std::to_string(i) + " endpoint B.");

        if (w.length_cm <= SNAP_EPS_CM) {
            errors.push_back({"Wall " + std::to_string(i) + " has a zero or near-zero length."});
        }

        if (w.layers.empty()) {
            errors.push_back({"Wall " + std::to_string(i) + " has no material layers."});
        }

        for (const auto& layer : w.layers) {
            if (layer.thickness_cm <= 0.0) {
                errors.push_back({"Wall " + std::to_string(i) + " has non-positive layer thickness."});
            }

            if (layer.material_id < 0) {
                errors.push_back({"Wall " + std::to_string(i) + " has invalid layer material_id."});
            }
        }

        // check openings on wall
        for (std::size_t j = 0; j < w.openings.size(); ++j) {
            const auto& o = w.openings[j];

            if (o.length_cm <= 0.0) { // not possible to have negative length
                errors.push_back({"Opening " + std::to_string(j) + " on wall " + std::to_string(i) + " has non-positive length."});
            }

            if (o.center_t < 0.0 || o.center_t > 1.0) {
                errors.push_back({"Opening " + std::to_string(j) + " on wall " + std::to_string(i) + " has center_t outside [0,1]."});
            }

            if (w.length_cm > 0.0) {
                double half_t = (o.length_cm * 0.5) / w.length_cm;
                if (o.center_t - half_t < 0.0 || o.center_t + half_t > 1.0) {
                    errors.push_back({"Opening " + std::to_string(j) + " on wall " + std::to_string(i) + " extends beyond wall bounds."});
                }
            }
        }

        // check for overlap on openings
        for (std::size_t a = 0; a < w.openings.size(); ++a) {
            const auto& oa = w.openings[a];
            double half_t_a = (oa.length_cm * 0.5) / w.length_cm;
            double a_start = oa.center_t - half_t_a;
            double a_end = oa.center_t + half_t_a;

            for (std::size_t b = a + 1; b < w.openings.size(); ++b) {
                const auto& ob = w.openings[b];
                double half_t_b = (ob.length_cm * 0.5) / w.length_cm;
                double b_start = ob.center_t - half_t_b;
                double b_end = ob.center_t + half_t_b;

                // check for overlap
                if (std::max(a_start, b_start) < std::min(a_end, b_end)) {
                    errors.push_back({"Openings " + std::to_string(a) + " and " + std::to_string(b) + " overlap on wall " + std::to_string(i) + "."});
                }
            }
        }
    }

    // entities
    for (std::size_t i = 0; i < entities.size(); ++i) {
        const auto& e = entities[i];

        // check if entities are in bounds
        if (e.position.x_cm < 0.0 || e.position.x_cm > world_bounds.width_cm || e.position.y_cm < 0.0 || e.position.y_cm > world_bounds.height_cm) {
            errors.push_back({"Entity " + std::to_string(i) + " is outside grid bounds."});
        }

        if (!std::isfinite(e.position.x_cm) || !std::isfinite(e.position.y_cm)) {
            errors.push_back({"Entity " + std::to_string(i) + " has invalid coordinates."});
        }
    }

    return errors;
}


void GeometryEngine::clear() {
    walls.clear();
    points.clear();
}


const WorldBounds& GeometryEngine::get_world_bounds() const { return world_bounds; }


void GeometryEngine::set_world_bounds(const WorldBounds& bounds) {
    world_bounds = bounds;
}


void GeometryEngine::set_world_bounds_from_image(int px_w, int px_h) {
    world_bounds.width_cm = static_cast<double>(px_w);
    world_bounds.height_cm = static_cast<double>(px_h);

    cm_per_cell = world_bounds.width_cm / grid_cells;
}