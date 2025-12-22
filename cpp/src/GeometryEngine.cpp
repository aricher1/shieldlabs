#include "geometry/GeometryEngine.hpp"
#include <nlohmann/json.hpp>
#include <cmath>
#include <map>

using json = nlohmann::json;

constexpr double SNAP_EPS_CM = 1e-3;


namespace {  
    
    // =============== helper functions for find_intersections() =================
    static bool segment_intersection(const Point& a1, const Point& a2, const Point& b1, const Point& b2, double& ta, double& tb, Point& out) {

        double x1 = a1.x_cm, y1 = a1.y_cm;
        double x2 = a2.x_cm, y2 = a2.y_cm;
        double x3 = b1.x_cm, y3 = b1.y_cm;
        double x4 = b2.x_cm, y4 = b2.y_cm;

        double dx1 = x2 - x1;
        double dy1 = y2 - y1;
        double dx2 = x4 - x3;
        double dy2 = y4 - y3;

        double denom = dx1 * dy2 - dy1 * dx2;
        if (std::abs(denom) < 1e-9) {
            return false;   // parallel
        }

        double dx3 = x3 - x1;
        double dy3 = y3 - y1;

        ta = (dx3 * dy2 - dy3 * dx2) / denom;
        tb = (dx3 * dy1 - dy3 * dx1) / denom;

        if (ta <= 1e-6 || ta >= 1.0 - 1e-6) {
            return false;
        }
        if (tb <= 1e-6 || tb >= 1.0 - 1e-6) {
            return false;
        }

        out = { x1 + ta * dx1, y1 + ta * dy1 };

        return true;
    }

    
    static bool endpoint_on_segment(const Point& p, const Point& a, const Point& b, double& t) {

        double dx = b.x_cm - a.x_cm;
        double dy = b.y_cm - a.y_cm;
        double len2 = dx * dx + dy * dy;

        if (len2 < 1e-9) {
            return false;
        }

        // project p onto a->b
        t = ((p.x_cm - a.x_cm) * dx + (p.y_cm - a.y_cm) * dy) / len2;

        if (t <= 1e-6 || t >= 1.0 - 1e-6) {
            return false;
        }

        // closest point
        double px = a.x_cm + t * dx;
        double py = a.y_cm + t * dy;

        // check collinearity
        double dist = std::hypot(px - p.x_cm, py - p.y_cm);

        return dist < 1e-6;
    } 
    
    static bool same_point(const Point& a, const Point& b) {

        constexpr double EPS = 1e-6;

        return std::hypot(a.x_cm - b.x_cm, a.y_cm - b.y_cm) < EPS;

    }

    static bool endpoint_on_segment(const Point& p, const Point& a, const Point& b) {

        const double dx = b.x_cm - a.x_cm;
        const double dy = b.y_cm - a.y_cm;
        const double len2 = dx * dx + dy * dy;

        if (len2 < 1e-9) { 
            return false; 
        }

        const double t = ((p.x_cm - a.x_cm) * dx + (p.y_cm - a.y_cm) * dy) / len2;

        if (t <= 0.0 || t >= 1.0) {
            return false;
        }

        const double px = a.x_cm + t * dx;
        const double py = a.y_cm + t * dy;

        constexpr double EPS = 1e-6;

        return std::hypot(px - p.x_cm, py - p.y_cm) < EPS;

    } 
    // ================ end of helpers for find_intersections() ==================

    // ================== helper functions for resolve_intersections() ===================
    static Point lerp(const Point& a, const Point& b, double t) {

        return {a.x_cm + t * (b.x_cm - a.x_cm), a.y_cm + t * (b.y_cm - a.y_cm)};

    } 

} // end namespace


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


void GeometryEngine::remove_entity_at(std::size_t index) {

    if (index < entities.size()) {

        entities.erase(entities.begin() + index);

    }
}


void GeometryEngine::add_entitiy_direct(const PointEntity& e) {

    entities.push_back(e);

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


std::vector<WallIntersection> GeometryEngine::find_intersections() const {

    std::vector<WallIntersection> result;

    for (size_t i = 0; i < walls.size(); ++i) {
        for (size_t j = i + 1; j < walls.size(); ++j) {

            const auto& wi = walls[i];
            const auto& wj = walls[j];

            double ti, tj;
            Point ip;

            // 1) Proper interior-interior crossing (X)
            if (segment_intersection(wi.a, wi.b, wj.a, wj.b, ti, tj, ip)) {
                result.push_back({i, j, ti, tj, snap_to_grid(ip)});
            }

            // 2) Endpoint of wi touches interior of wj
            if (endpoint_on_segment(wi.a, wj.a, wj.b)) {
                result.push_back({i, j, 0.0, -1.0, wi.a});
            }
            if (endpoint_on_segment(wi.b, wj.a, wj.b)) {
                result.push_back({i, j, 1.0, -1.0, wi.b});
            }

            // 3) Endpoint of wj touches interior of wi
            if (endpoint_on_segment(wj.a, wi.a, wi.b)) {
                result.push_back({j, i, 0.0, -1.0, wj.a});
            }
            if (endpoint_on_segment(wj.b, wi.a, wi.b)) {
                result.push_back({j, i, 1.0, -1.0, wj.b});
            }

            // 4) Endpoint-endpoint (corner or continuation)
            if (same_point(wi.a, wj.a) || same_point(wi.a, wj.b)) {
                result.push_back({i, j, 0.0, 0.0, wi.a});
            }
            if (same_point(wi.b, wj.a) || same_point(wi.b, wj.b)) {
                result.push_back({i, j, 1.0, 1.0, wi.b});
            }
        }
    }

    return result;
}


/* has bug needs fixed
void GeometryEngine::resolve_intersections() {

    auto intersections = find_intersections();
    
    if (intersections.empty()) {
        return;
    }

    std::vector<std::vector<double>> cuts(walls.size());

    for (size_t i = 0; i < walls.size(); ++i) {
        cuts[i].push_back(0.0);
        cuts[i].push_back(1.0);
    }

    for (const auto& is : intersections) {

        if (is.type != IntersectionType::Cross) {
            continue;
        }

        cuts[is.wall_i].push_back(is.ti);
        cuts[is.wall_j].push_back(is.tj);

    }

    std::vector<Wall> new_walls;

    for (size_t i = 0; i < walls.size(); ++i) {

        auto& c = cuts[i];
        std::sort(c.begin(), c.end());

        c.erase(std::unique(c.begin(), c.end(),
            [](double a, double b) {
                return std::abs(a - b) < 1e-6;
            }), c.end());

        const auto& w = walls[i];

        for (size_t k = 0; k + 1 < c.size(); ++k) {

            double t0 = c[k];
            double t1 = c[k + 1];
            
            if (t1 - t0 < 1e-6) {
                continue;
            }

            Point p0 = snap_to_grid(lerp(w.a, w.b, t0));
            Point p1 = snap_to_grid(lerp(w.a, w.b, t1));

            p0 = reuse_or_add(p0);
            p1 = reuse_or_add(p1);

            double len = std::hypot(p1.x_cm - p0.x_cm, p1.y_cm - p0.y_cm);

            if (len < SNAP_EPS_CM) {
                continue;
            }

            new_walls.push_back({p0, p1, w.thickness_cm, w.material_id, len});
        }
    }

    walls = std::move(new_walls);
}
*/


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


ConnectivityResult GeometryEngine::validate_connectivity() const {

    ConnectivityResult result;
    result.ok = true;

    // count how many walls touch each point
    struct PointKey {
        double x;
        double y;

        bool operator<(const PointKey& other) const {
            if (x != other.x) {
                return x < other.x;
            } else {
                return y < other.y;
            }
        }
    };

    std::map<PointKey, int> degree_map;

    for (const auto& w : walls) {

        PointKey ka{w.a.x_cm, w.a.y_cm};
        PointKey kb{w.b.x_cm, w.b.y_cm};

        degree_map[ka]++;
        degree_map[kb]++;
    }

    // find dangling points
    for (const auto& [key, degree] : degree_map) {

        if (degree < 2) {
            result.ok = false;
            result.dangling_points.push_back({{key.x, key.y}, degree});
        }
    }
    
    return result;
}


void GeometryEngine::clear() {

    walls.clear();
    points.clear();

}