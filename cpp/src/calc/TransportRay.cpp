#include "calc/TransportRay.hpp"
#include "calc/CalcScene.hpp"
#include "calc/RayTraversal.hpp"
#include "calc/RayHitProcessing.hpp"
#include "calc/HitClassification.hpp"
#include <cmath>
#include <algorithm>


namespace calc {

TransportRay build_transport_ray(const CalcScene& scene, int source_index, int dose_index) {
    const auto& src = scene.sources[source_index].position;
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

    // normalize ray direction
    const double inv_len = 1.0 / std::max(total_distance_cm, 1e-9);
    const double ray_dx = dx * inv_len;
    const double ray_dy = dy * inv_len;

    for (const auto& hit : hits) {
        // air before hit
        const double air_len = hit.distance_cm - prev_dist;
        if (air_len > 0.0) {
            ray.segments.push_back({-1, air_len}); // -1 for air
        }

        double wall_transport_len = 0.0;

        // solid wall
        if (hit.kind == HitKind::SolidWall) {
            const CalcWall& wall = scene.walls[hit.wall_index];

            const double wx = wall.b.x_cm - wall.a.x_cm;
            const double wy = wall.b.y_cm - wall.a.y_cm;
            const double wlen = std::hypot(wx, wy);

            if (wlen > 0.0) {
                // wall normal perpendicular
                const double nx = -wy / wlen;
                const double ny = wx / wlen;
            
                // cosine of incidence angle
                double cos_incidence = std::fabs(ray_dx * nx + ray_dy * ny);
                cos_incidence = std::max(cos_incidence, 1e-6);

                for (const auto& layer : wall.layers) {
                    const double seg_len = layer.thickness_cm / cos_incidence;
                    ray.segments.push_back({layer.material_id, seg_len});
                    wall_transport_len += seg_len;
                }
            }
        }

        prev_dist = hit.distance_cm + wall_transport_len;
    }

    const double tail = total_distance_cm - prev_dist;
    if (tail > 0.0) {
        ray.segments.push_back({-1, tail});
    }

    return ray;
}


} // end namespace calc