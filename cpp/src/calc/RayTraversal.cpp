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
            bool intersects = intersect_ray_with_segment(
                sx, sy,
                dx, dy,
                wall.a.x_cm, wall.a.y_cm,
                wall.b.x_cm, wall.b.y_cm,
                hit
            );
            if (!intersects) { continue; }

            RayHitRecord record;
            record.wall_index = static_cast<int>(i);
            record.t_ray = hit.t_ray;
            record.distance_cm = ray_distance(sx, sy, dx, dy, hit.t_ray);
            record.kind = classify_wall_hit(wall, hit.t_wall);

            hits.push_back(record);
        }

        return hits;
    }

} // end namespace calc