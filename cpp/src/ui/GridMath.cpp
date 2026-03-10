// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <algorithm>
#include <cmath>
#include <numbers>

#include "ui/GridMath.hpp"


constexpr double SNAP_POINT_EPS_CM = 0.01;      // snap epsilon, checking if p equals an existing endpoint for selection logic

namespace ui {

    double distance_point_to_segment(Point p, Point a, Point b) {
        const double dx = b.x_cm - a.x_cm;
        const double dy = b.y_cm - a.y_cm;

        if (dx == 0.0 && dy == 0.0) {
            return std::hypot(p.x_cm - a.x_cm, p.y_cm - a.y_cm);
        }

        const double t = ((p.x_cm - a.x_cm) * dx + (p.y_cm - a.y_cm) * dy) / (dx * dx + dy * dy);
        const double clamped = std::clamp(t, 0.0, 1.0);
        const double proj_x = a.x_cm + clamped * dx;
        const double proj_y = a.y_cm + clamped * dy;

        return std::hypot(p.x_cm - proj_x, p.y_cm - proj_y);
    }


    double distance_point_to_point(Point a, Point b) {
        return std::hypot(a.x_cm - b.x_cm, a.y_cm - b.y_cm);
    }


    bool snaps_to_existing_point(const Point& p, const geom::GeometryEngine& engine) {
        for (const auto& w : engine.get_walls()) {
            if (std::hypot(p.x_cm - w.a.x_cm, p.y_cm - w.a.y_cm) < SNAP_POINT_EPS_CM) {
                return true;
            }
            if (std::hypot(p.x_cm - w.b.x_cm, p.y_cm - w.b.y_cm) < SNAP_POINT_EPS_CM) {
                return true;
            }
        }
        
        return false;
    }


    double project_t_onto_wall(const Point& p, const geom::Wall& w) {
        double dx = w.b.x_cm - w.a.x_cm;
        double dy = w.b.y_cm - w.a.y_cm;
        double len2 = dx * dx + dy * dy;
        if (len2 < 1e-9) {
            return 0.0;
        }
        double t = ((p.x_cm - w.a.x_cm) * dx + (p.y_cm- w.a.y_cm) * dy) / len2;

        return std::clamp(t, 0.0, 1.0);
    }


    Point lerp_point(const Point& a, const Point& b, double t) {
        return {a.x_cm + t * (b.x_cm - a.x_cm), a.y_cm + t * (b.y_cm - a.y_cm)};
    }


    sf::RectangleShape make_thick_segment(sf::Vector2f a, sf::Vector2f b, float thickness, sf::Color color) {

        sf::Vector2f d = b - a;
        float length = std::sqrt(d.x * d.x + d.y * d.y);
        sf::RectangleShape r({length, thickness});
        r.setFillColor(color);
        r.setOrigin(sf::Vector2f{0.f, thickness * 0.5f});
        r.setPosition(a);
        r.setRotation(sf::degrees(std::atan2(d.y, d.x) * 180.f / static_cast<float>(std::numbers::pi)));
            
        return r;
    }

} // end namespace ui
