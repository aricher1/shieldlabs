#pragma once
#include "Point.hpp"
#include "Wall.hpp"
#include "PointEntity.hpp"
#include "WorldBounds.hpp"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>




class GeometryEngine {

    private:
        WorldBounds world_bounds;
        int grid_cells;
        double cm_per_cell;

        std::vector<Point> points;
        std::vector<Wall> walls;

        std::vector<PointEntity> entities;

        Point reuse_or_add(Point p);

    public:
        explicit GeometryEngine(int grid_cells, double cm_per_cell);

        const WorldBounds& get_world_bounds() const;
        void set_world_bounds(const WorldBounds& bounds);
        void set_world_bounds_from_image(int px_w, int px_h);

        // void apply_scale(double measured_cm, double real_cm);
    
        int get_grid_cells() const { return grid_cells; }
        double get_cm_per_cell() const { return cm_per_cell; }
        void set_scale(int cells, double cm);
        std::vector<Wall>& get_walls_mutable() { return walls; }

        Point add_point(Point p);
        Point snap_to_grid(Point p) const;

        void add_wall(Point a, Point b, double thickness_cm, int material_id);
        void add_wall_direct(const Wall& w);

        void remove_last_wall();
        void remove_wall_at(std::size_t index);
        void remove_entity_at(std::size_t index);
        void add_entity_direct(const PointEntity& e);
        const std::vector<Wall>& get_walls() const;

        void add_source(Point p);
        void add_dose(Point p);
        const std::vector<PointEntity>& get_entities() const;
        std::vector<PointEntity>& get_entities_mutable() { return entities; }

        nlohmann::json to_json() const;
        bool load_from_json(const std::string& json_str);

        // error handling for validate() func
        struct ValidationError {
            std::string message;
        };
        std::vector<ValidationError> validate() const;
        
        void clear();        
};