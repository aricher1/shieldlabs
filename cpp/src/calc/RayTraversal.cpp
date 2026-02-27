// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include "calc/RayTraversal.hpp"
#include "calc/RayIntersection.hpp"
#include "calc/GeometryUtils.hpp"
#include "calc/HitClassification.hpp"


namespace calc {

    std::vector<RayHitRecord> trace_ray(const CalcScene& scene, int source_index, int dose_index) {
        std::vector<RayHitRecord> hits; // vector to store all hits
        const auto& source = scene.sources[source_index];
        const auto& dose = scene.dose_points[dose_index];
        const double sx = source.position.x_cm;
        const double sy = source.position.y_cm;
        const double dx = dose.position.x_cm;
        const double dy = dose.position.y_cm;

        for (std::size_t i = 0; i < scene.walls.size(); ++i) {
            const auto& wall = scene.walls[i];

            RayHit hit;
            if (!intersect_ray_with_segment(
                    sx, sy,
                    dx, dy,
                    wall.a.x_cm, wall.a.y_cm,
                    wall.b.x_cm, wall.b.y_cm,
                    hit)) { 
                continue; 
            }

            const double mid_dist = ray_distance(sx, sy, dx, dy, hit.t_ray);
            double total_thickness = 0.0;
            for (const auto& layer : wall.layers) {
                total_thickness += layer.thickness_cm;
            }

            RayHitRecord entry;
            entry.wall_index = static_cast<int>(i);
            entry.distance_cm = mid_dist - total_thickness * 0.5;
            entry.kind = HitKind::SolidWall;

            RayHitRecord exit;
            exit.wall_index = static_cast<int>(i);
            exit.distance_cm = mid_dist + total_thickness * 0.5;
            exit.kind = HitKind::SolidWall;

            if (exit.distance_cm > entry.distance_cm) {
                hits.push_back(entry);
                hits.push_back(exit);
            }

        }

        return hits;
    }

} // end namespace calc