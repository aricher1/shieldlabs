// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <cmath>


namespace calc {

    // Parametric intersection data for a ray-segment hit
    struct RayHit {
        double t_ray;   // [0,1] along source -> dose
        double t_wall;  // [0,1] along wall a -> b
    };

    // Computes the intersection between a ray and a wall segment
    // Reutrns true if the intersection lies within both wall segments
    bool intersect_ray_with_segment (
        double rx0, double ry0,
        double rx1, double ry1,
        double wx0, double wy0,
        double wx1, double wy1,
        RayHit& hit
    );

} // end namespace calc