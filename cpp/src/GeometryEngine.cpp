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

    j["sources"] = json::array();           // CT machines, bathrooms, etc.
    j["evaluation_points"] = json::array(); // humans, offices, waiting rooms

    return j.dump(2);
}