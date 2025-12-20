#pragma once

#include "Point.hpp"
#include "Wall.hpp"
#include <vector>
#include <string>


class GeometryEngine {

    private:
        int grid_cells;
        double cm_per_cell;

        std::vector<Point> points;
        std::vector<Wall> walls;

        Point snap_to_grid(Point p) const;
        Point reuse_or_add(Point p);

    public:
        explicit GeometryEngine(int grid_cells, double cm_per_cell);
    
        int get_grid_cells() const { return grid_cells; }
        double get_cm_per_cell() const { return cm_per_cell; }

        void set_scale(int cells, double cm);

        Point add_point(Point p);

        void add_wall(Point a, Point b, double thickness_cm, int material_id);
        void add_wall_direct(const Wall& w);

        void remove_last_wall();
        void remove_wall_at(std::size_t index);

        const std::vector<Wall>& get_walls() const;

        std::string to_json() const;

};
