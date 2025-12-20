#include "geometry/GeometryEngine.hpp"
#include <nlohmann/json.hpp>
#include <cmath>

using json = nlohmann::json;

constexpr double SNAP_EPS_CM = 1e-3;

GeometryEngine::GeometryEngine(int cells, double cm) : grid_cells(cells), cm_per_cell(cm) {}


Point GeometryEngine::snap_to_grid(Point p) const {
    
    return {
        std::round(p.x_cm / cm_per_cell) * cm_per_cell,
        std::round(p.y_cm / cm_per_cell) * cm_per_cell
    };

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

    walls.push_back({a, b, thickness_cm, material_id, length_cm});

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


const std::vector<Wall>& GeometryEngine::get_walls() const { return walls; }


void GeometryEngine::add_source(Point p) {

    p = snap_to_grid(p);
    entities.push_back({p, PointType::Source, ""});
     
}


void GeometryEngine::add_dose(Point p) {

    p = snap_to_grid(p);
    entities.push_back({p, PointType::Dose, ""});

}


const std::vector<PointEntity>& GeometryEngine::get_entities() const { return entities; }


std::string GeometryEngine::to_json() const {

    json j;

    j["version"] = 1;
    j["units"] = "cm";

    j["grid"] = {
        {"cells", grid_cells},
        {"cm_per_cell", cm_per_cell}
    };

    j["walls"] = json::array();
    
    for (const auto& w : walls) {
        j["walls"].push_back({
            {"a", {{"x", w.a.x_cm}, {"y", w.a.y_cm}}},
            {"b", {{"x",w.b.x_cm}, {"y", w.b.y_cm}}},
            {"thickness_cm", w.thickness_cm},
            {"material_id", w.material_id}
        });
    }

    j["sources"] = json::array();
    j["dose_points"] = json::array();

    for (const auto& e : entities) {

        json entry = {{"x", e.position.x_cm}, {"y", e.position.y_cm}};

        if (!e.label.empty()) {

            entry["label"] = e.label;

        }

        if (e.type == PointType::Source) {

            j["sources"].push_back(entry);

        } else {

            j["dose_points"].push_back(entry);

        }
    }

    return j.dump(2);
}


bool GeometryEngine::load_from_json(const std::string& json_str) {

    json j;
    try {

        j = json::parse(json_str);

    } catch (...) {

        return false;

    }

    if (!j.contains("version") || j["version"] != 1) { return false; }
    if (!j.contains("grid")) { return false; }

    grid_cells  = j["grid"]["cells"];
    cm_per_cell = j["grid"]["cm_per_cell"];
    clear();
    entities.clear();

    if (j.contains("walls")) {

        for (const auto& jw : j["walls"]) {

            Point a{jw["a"]["x"], jw["a"]["y"]};
            Point b{jw["b"]["x"], jw["b"]["y"]};

            double thickness = jw["thickness_cm"];
            int material_id  = jw["material_id"];

            add_wall(a, b, thickness, material_id);

        }
    }

    if (j.contains("sources")) {

        for (const auto& s : j["sources"]) {
            
            entities.push_back({{s["x"], s["y"]}, PointType::Source, s.value("label", "")});
        
        }
    }

    if (j.contains("dose_points")) {

        for (const auto& d : j["dose_points"]) {

            entities.push_back({{d["x"], d["y"]}, PointType::Dose, d.value("label", "")});

        }
    }

    return true;

}


void GeometryEngine::clear() {

    walls.clear();
    points.clear();

}