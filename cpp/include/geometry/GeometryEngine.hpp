#pragma once

#include "Point.hpp"
#include "Wall.hpp"
#include <vector>
#include <string>


class GeometryEngine {

    private:
        double grid_spacing_cm;
        std::vector<Point> points;
        std::vector<Wall> walls;

        Point snap_to_grid(Point p) const;
        Point reuse_or_add(Point p);


    public:
        explicit GeometryEngine(double grid_spacing_cm = 1.0);

        Point add_point(Point p);

        void add_wall(Point a, Point b, double thickness_cm, int material_id);

        void add_wall_direct(const Wall& w);
        void remove_last_wall();
        void remove_wall_at(std::size_t index);

        const std::vector<Wall>& get_walls() const;

        std::string to_json() const;

        double get_grid_spacing_cm() const { return grid_spacing_cm; }

};
