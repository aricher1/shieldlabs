// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <cmath>
#include <algorithm>

#include "calc/TransportRay.hpp"
#include "calc/CalcScene.hpp"
#include "calc/RayTraversal.hpp"
#include "calc/RayHitProcessing.hpp"



namespace calc {

    TransportRay build_transport_ray(const CalcScene& scene, int source_index, int dose_index) {
        const auto& src  = scene.sources[source_index].position;
        const auto& dose = scene.dose_points[dose_index].position;

        const double dx = dose.x_cm - src.x_cm;
        const double dy = dose.y_cm - src.y_cm;
        const double total_distance_cm = std::hypot(dx, dy);

        TransportRay ray;
        ray.source_index = source_index;
        ray.dose_index = dose_index;
        ray.geometric_distance_cm = total_distance_cm;

        auto hits = trace_ray(scene, source_index, dose_index);
        sort_hits_by_distance(hits);

        double prev_dist = 0.0;

        // ray direction
        const double inv_len = 1.0 / std::max(total_distance_cm, 1e-9);
        const double rdx = dx * inv_len;
        const double rdy = dy * inv_len;

        for (std::size_t i = 0; i + 1 < hits.size(); i += 2) {
            const auto& entry = hits[i];
            const auto& exit = hits[i + 1];
            const int wall_index = entry.wall_index;

            // air before wall
            const double air_len = entry.distance_cm - prev_dist;
            if (air_len > 0.0) {
                ray.segments.push_back({-1, -1, air_len});
            }

            const CalcWall& wall = scene.walls[wall_index];

            const double wx = wall.b.x_cm - wall.a.x_cm;
            const double wy = wall.b.y_cm - wall.a.y_cm;
            const double wlen = std::hypot(wx, wy);

            if (wlen > 0.0) {
                const double nx = -wy / wlen;
                const double ny =  wx / wlen;

                double cos_incidence = std::fabs(rdx * nx + rdy * ny);
                cos_incidence = std::max(cos_incidence, 1e-6);

                for (const auto& layer : wall.layers) {
                    ray.segments.push_back({
                        wall_index,
                        layer.material_id,
                        layer.thickness_cm / cos_incidence
                    });
                }
            }

            prev_dist = exit.distance_cm;
        }

        // tail air
        const double tail = total_distance_cm - prev_dist;
        if (tail > 0.0) {
            ray.segments.push_back({-1, -1, tail});
        }

        return ray;
    }

} // namespace calc