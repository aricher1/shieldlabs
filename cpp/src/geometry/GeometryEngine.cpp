// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <nlohmann/json.hpp>
#include <cmath>
#include <map>

#include "geometry/GeometryEngine.hpp"

using json = nlohmann::json;

constexpr double SNAP_EPS_CM = 1e-3;



namespace geom {

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
            .uptake_time_hours = 0.0f,
            .apply_patient_attenuation = false,
            .patient_attenuation_percent = 0.0f,
            .apply_radioactive_decay = true
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


    void GeometryEngine::set_selected_isotope_key(const std::string& key) {
        selected_isotope_key = key;
    }


    const std::vector<PointEntity>& GeometryEngine::get_entities() const { return entities; }


    nlohmann::json GeometryEngine::to_json() const {
        // all geometry is stored in raw grid units.
        // distance_scale is applied only for display and calculations.
        // never scale or snap during save/load.
        json j;

        j["version"] = 2;
        j["units"] = "cm";

        j["grid"] = {
            {"cells", grid_cells},
            {"cm_per_cell", cm_per_cell}
        };

        j["distance_scale"] = distance_scale;
        j["isotope"] = selected_isotope_key;
        j["walls"] = json::array();
        
        for (const auto& w : walls) {
            
            json jw;
            jw["a"] = {{"x", w.a.x_cm}, {"y", w.a.y_cm}};
            jw["b"] = {{"x", w.b.x_cm}, {"y", w.b.y_cm}};
            jw["layers"] = json::array();
            for (const auto& layer : w.layers) {
                jw["layers"].push_back({{"material_id", layer.material_id}, {"thickness_cm", layer.thickness_cm}});
            }
            jw["length_cm"] = w.length_cm;

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
                entry["patient_attenuation_percent"] = s.patient_attenuation_percent;
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


    bool GeometryEngine::load_from_json(const nlohmann::json& j) {

        if (!j.contains("version")) { return false; }
        int version = j["version"];
        if (version != 1 && version != 2) { return false; }
        if (!j.contains("grid")) { return false; }

        clear();
        entities.clear();

        grid_cells = j["grid"]["cells"];
        cm_per_cell = j["grid"]["cm_per_cell"];
        distance_scale = j.value("distance_scale", 1.0);
        selected_isotope_key = j.value("isotope", "f18");

        // walls
        if (j.contains("walls")) {
            for (const auto& jw : j["walls"]) {

                Point a{jw["a"]["x"], jw["a"]["y"]};
                Point b{jw["b"]["x"], jw["b"]["y"]};

                Wall w;
                w.a = reuse_or_add(a);
                w.b = reuse_or_add(b);
                w.length_cm = std::hypot(w.a.x_cm - w.b.x_cm, w.a.y_cm - w.b.y_cm);

                if (jw.contains("layers")) {
                    for (const auto& jl : jw["layers"]) {
                        w.layers.push_back({
                            jl["material_id"],
                            jl["thickness_cm"]
                        });
                    }
                }

                walls.push_back(w);
            }
        }

        // sources
        if (j.contains("sources")) {
            for (const auto& s : j["sources"]) {
                PointEntity e;
                e.position = {s["x"], s["y"]};
                e.type = PointType::Source;
                e.label = s.value("label", "");
                e.source = SourceData{
                    .num_patients = s.value("num_patients", 1.0f),
                    .activity_per_patient_MBq = s.value("activity_per_patient_MBq", 0.0f),
                    .uptake_time_hours = s.value("uptake_time_hours", 0.0f),
                    .apply_patient_attenuation = s.value("apply_patient_attenuation", true),
                    .patient_attenuation_percent = s.value("patient_attenuation_percent", 0.0f),
                    .apply_radioactive_decay = s.value("apply_radioactive_decay", true)
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

            // all point entities must have a name
            if (e.label.empty()) {
                errors.push_back({"Unnamed source or dose."});
                continue;
            }

            // source info validation
            if (e.type == PointType::Source) {

                if (!e.source.has_value()) {
                    errors.push_back({"Source [" + e.label + "] has no source data."});
                } else {
                    const auto& s = *e.source;

                    if (s.num_patients < 0.0f) {
                        errors.push_back({"Source [" + e.label + "] has non-positive patient count."});
                    }

                    if (s.activity_per_patient_MBq <= 0.0f) {
                        errors.push_back({"Source [" + e.label + "] has non-positive activity per patient."});
                    }

                    if (s.uptake_time_hours < 0.0f) {
                        errors.push_back({"Source [" + e.label + "] has negative uptake time."});
                    }
                }
            }

            // dose info validation
            if (e.type == PointType::Dose) {

                if (!e.dose.has_value()) {
                    errors.push_back({"Dose point [" + e.label + "] has no dose data."});
                } else {
                    const auto& d = *e.dose;

                    if (d.occupancy < 0.0f || d.occupancy > 1.0f) {
                        errors.push_back({"Dose point [" + e.label + "] has occupancy outside [0,1]."});
                    }

                    if (d.dose_limit_uSv <= 0.0f) {
                        errors.push_back({"Dose point [" + e.label + "] has non-positive dose limit."});
                    }
                }
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

} // end namespace geom