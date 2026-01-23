#pragma once
#include <SFML/Graphics.hpp>
#include "geometry/GeometryEngine.hpp"


// Grid math helpers
double distance_point_to_segment(Point p, Point a, Point b);


double distance_point_to_point(Point a, Point b);


double project_t_onto_wall(const Point& p, const Wall& w);


Point lerp_point(const Point& a, const Point& b, double t);


sf::RectangleShape make_thick_segment(sf::Vector2f a, sf::Vector2f b, float thickness, sf::Color color);