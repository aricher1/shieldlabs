// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <nlohmann/json.hpp>
#include <vector>
#include <string>

#include "Point.hpp"
#include "Wall.hpp"
#include "PointEntity.hpp"
#include "WorldBounds.hpp"



namespace geom {

    class GeometryEngine {

        private:
            WorldBounds world_bounds;
            int grid_cells;
            double cm_per_cell;
            double distance_scale = 1.0;
            std::string selected_isotope_key = "f18";
            std::vector<Point> points;
            std::vector<Wall> walls;
            std::vector<PointEntity> entities;

            Point reuse_or_add(Point p);

        public:
            explicit GeometryEngine(int grid_cells, double cm_per_cell);

            // World configuration
            const WorldBounds& get_world_bounds() const;
            void set_world_bounds(const WorldBounds& bounds);
            void set_world_bounds_from_image(int px_w, int px_h);

            // Getters
            int get_grid_cells() const { return grid_cells; }
            double get_cm_per_cell() const { return cm_per_cell; }
            double get_distance_scale() const { return distance_scale; }
            const std::vector<Wall>& get_walls() const;
            std::vector<Wall>& get_walls_mutable() { return walls; }
            const std::vector<PointEntity>& get_entities() const;
            std::vector<PointEntity>& get_entities_mutable() { return entities; }
            const std::string& get_selected_isotope_key() const { return selected_isotope_key; }

            // Setters
            void set_distance_scale(double scale);
            void set_selected_isotope_key(const std::string& key);
            void set_scale(int cells, double cm);

            // Add
            Point add_point(Point p);
            Point snap_to_grid(Point p) const;
            void add_wall(Point a, Point b, double thickness_cm, int material_id);
            void add_wall_direct(const Wall& w);
            void add_entity_direct(const PointEntity& e);
            void add_source(Point p);
            void add_dose(Point p);

            // Remove
            void remove_last_wall();
            void remove_wall_at(std::size_t index);
            void remove_entity_at(std::size_t index);

            // JSON
            nlohmann::json to_json() const;
            bool load_from_json(const nlohmann::json& j);

            // Validation
            struct ValidationError {
                std::string message;
            };
            std::vector<ValidationError> validate() const;
            
            // Clear walls and points
            void clear();        
    };

} // end namespace geom