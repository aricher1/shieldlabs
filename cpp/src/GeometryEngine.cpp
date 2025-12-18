#include "geometry/GeometryEngine.hpp"
#include <nlohmann/json.hpp>
#include <cmath>

using json = nlohmann::json;

constexpr double SNAP_EPS_CM = 1e-3;

GeometryEngine::GeometryEngine(double grid_spacing_cm) : grid_spacing_cm(grid_spacing_cm) {}


Point GeometryEngine::snap_to_grid(Point p) const {
    
    return {
        std::round(p.x_cm / grid_spacing_cm) * grid_spacing_cm,
        std::round(p.y_cm / grid_spacing_cm) * grid_spacing_cm
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

    if (std::hypot(a.x_cm - b.x_cm, a.y_cm - b.y_cm) < SNAP_EPS_CM) {
        return;
    }

    walls.push_back({a, b, thickness_cm, material_id});

}


const std::vector<Wall>& GeometryEngine::get_walls() const { return walls; }


std::string GeometryEngine::to_json() const {

    json j;
    j["units"] = "cm";

    j["walls"] = json::array();
    
    for (const auto& w : walls) {
        j["walls"].push_back({
            {"a", {w.a.x_cm, w.a.y_cm}},
            {"b", {w.b.x_cm, w.b.y_cm}},
            {"thickness", w.thickness_cm},
            {"material_id", w.material_id}
        });
    }

    j["sources"] = json::array();
    j["evaluation_points"] = json::array();

    return j.dump(2);
}