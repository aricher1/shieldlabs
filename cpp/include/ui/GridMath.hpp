// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <SFML/Graphics.hpp>

#include "geometry/GeometryEngine.hpp"



namespace ui {
        
    // Distance utilities
    double distance_point_to_segment(Point p, Point a, Point b);
    double distance_point_to_point(Point a, Point b);

    // Projection helper
    double project_t_onto_wall(const Point& p, const geom::Wall& w);

    // Interpolation
    Point lerp_point(const Point& a, const Point& b, double t);

    // Rendering helper
    sf::RectangleShape make_thick_segment(sf::Vector2f a, sf::Vector2f b, float thickness, sf::Color color);

} // end namespace ui